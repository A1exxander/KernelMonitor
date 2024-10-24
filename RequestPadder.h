#pragma once
#include <ntifs.h>
#include "Globals.h"

namespace RequestPadder {
    ULONG CalculatePaddingLength(ULONG InputSize);
    NTSTATUS AddPadding(PUCHAR pInput, ULONG InputSize, PUCHAR pOutput, ULONG PaddingLength, ULONG PaddedBufferSize);
    NTSTATUS RemovePadding(PUCHAR pBuffer, ULONG BufferSize, ULONG& UnpaddedBufferSize);
}