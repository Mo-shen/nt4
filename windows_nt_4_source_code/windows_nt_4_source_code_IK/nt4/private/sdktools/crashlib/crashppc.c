#define TARGET_PPC
#include <platform.h>
#include <crash.h>
#include <stdlib.h>
#include <string.h>

extern PDUMP_HEADER                    DumpHeader;
extern PUSERMODE_CRASHDUMP_HEADER      DumpHeaderUser;
extern PVOID                           DumpContext;
extern PKPRCB                          KiProcessors[];
extern ULONG                           KiPcrBaseAddress;
extern BOOL                            UserModeDump;


DmpReadControlSpacePPC(
    USHORT  Processor,
    PVOID   TargetBaseAddress,
    PVOID   UserInterfaceBuffer,
    ULONG   TransferCount,
    PULONG  ActualBytesRead
    )
{
    DWORD     cb;
    DWORD     StartAddr;


    StartAddr  = (DWORD)TargetBaseAddress +
                 (DWORD)KiProcessors[Processor]  +
                 (DWORD)&(((PKPRCB)0)->ProcessorState);

    cb = DmpReadMemory( (LPVOID)StartAddr, UserInterfaceBuffer, TransferCount );

    if (ActualBytesRead) {
        *ActualBytesRead = cb;
    }

    return cb > 0;
}


BOOL
DmpGetContextPPC(
    IN  ULONG     Processor,
    OUT PVOID     Context
    )
{
    DWORD     StartAddr;


    if (UserModeDump) {
        if (Processor > DumpHeaderUser->ThreadCount-1) {
            return FALSE;
        }
        CopyMemory( Context, &((PCONTEXT)DumpContext)[Processor], sizeof(CONTEXT) );
        return TRUE;
    }

    StartAddr  = (DWORD)KiProcessors[Processor]  +
                 (DWORD)&(((PKPRCB)0)->ProcessorState);

    return DmpReadMemory( (LPVOID)StartAddr, Context, sizeof(CONTEXT) );
}


INT
DmpGetCurrentProcessorPPC(
    VOID
    )
{
    ULONG   i;
    CONTEXT Context;


    for (i=0; i<DumpHeader->NumberProcessors; i++) {
        if (DmpGetContextPPC( i, &Context ) && Context.Gpr1 == ((PCONTEXT)DumpContext)->Gpr1) {
            return i;
        }
    }

    return -1;
}
