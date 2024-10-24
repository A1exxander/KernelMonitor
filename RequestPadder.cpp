#include "RequestPadder.h"

ULONG RequestPadder::CalculatePaddingLength(ULONG InputSize) { return AES_BLOCK_SIZE - (InputSize % AES_BLOCK_SIZE); }

NTSTATUS RequestPadder::AddPadding(PUCHAR pInput, ULONG InputSize, PUCHAR pOutput, ULONG PaddingLength, ULONG PaddedBufferSize) {

    if (!pInput || !pOutput || PaddedBufferSize == 0) { return STATUS_INVALID_PARAMETER; }
    RtlCopyMemory(pOutput, pInput, InputSize);

    for (ULONG i = InputSize; i < PaddedBufferSize; i++) {
        pOutput[i] = static_cast<UCHAR>(PaddingLength);
    }

    return STATUS_SUCCESS;
}

NTSTATUS RequestPadder::RemovePadding(PUCHAR pBuffer, ULONG BufferSize, ULONG& UnpaddedBufferSize) {

    if ((!pBuffer || BufferSize == 0 || !UnpaddedBufferSize) || BufferSize % AES_BLOCK_SIZE != 0) { return STATUS_INVALID_PARAMETER; }

    UCHAR PaddingLength = pBuffer[BufferSize - 1];

    if (PaddingLength == 0 || PaddingLength > AES_BLOCK_SIZE || PaddingLength > BufferSize) {
        return STATUS_DATA_ERROR;
    }

    for (UCHAR i = 1; i <= PaddingLength; i++) { // Since we are just checking, if we can ensure our data will be valid each time, we can remove this check to improve performance
        if (pBuffer[BufferSize - i] != PaddingLength) {
            return STATUS_DATA_ERROR;
        }
    }

    UnpaddedBufferSize = BufferSize - PaddingLength;
    return STATUS_SUCCESS;

}