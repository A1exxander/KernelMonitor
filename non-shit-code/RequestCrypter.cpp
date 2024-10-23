#include "RequestCrypter.h"

RequestCrypter::RequestCrypter() {
    UCHAR tempKey[16] = {
        0xB7, 0x4F, 0x3A, 0x6E, 0x9A, 0xC1, 0xF8, 0x3C,
        0x2D, 0x7E, 0xC4, 0x56, 0x71, 0x0A, 0x8B, 0x9D
    };
    RtlCopyMemory(m_aesKey, tempKey, sizeof(tempKey));
}

RequestCrypter::RequestCrypter(const UCHAR* aesKey) {
    RtlCopyMemory(m_aesKey, aesKey, 16 * sizeof(UCHAR));
}

RequestCrypter::~RequestCrypter() {
    if (hKey) { BCryptDestroyKey(hKey); }
    if (hAlg) { BCryptCloseAlgorithmProvider(hAlg, 0); }
}

void RequestCrypter::SetAESKey(const UCHAR* aesKey) {
    RtlCopyMemory(m_aesKey, aesKey, 16 * sizeof(UCHAR));
}

NTSTATUS RequestCrypter::Initialize() {

    if (hKey) {
        BCryptDestroyKey(hKey);
        hKey = NULL;
    }
    if (hAlg) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        hAlg = NULL;
    }

    NTSTATUS InitializationStatus = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0);
    if (!BCRYPT_SUCCESS(InitializationStatus)) {
        return InitializationStatus;
    }

    InitializationStatus = BCryptGenerateSymmetricKey(hAlg, &hKey, NULL, 0, m_aesKey, sizeof(m_aesKey), 0);
    if (!BCRYPT_SUCCESS(InitializationStatus)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        hAlg = NULL;
        return InitializationStatus;
    }

    return InitializationStatus;

}

NTSTATUS RequestCrypter::EncryptBuffer(PVOID pInput, ULONG InputSize, PVOID pOutput, ULONG OutputSize) { // Better to work w pVoid & not templates due to dynamic sizes

    if (!hKey || !hAlg) { return STATUS_INVALID_HANDLE; }
    if (!pInput || !pOutput) { return STATUS_INVALID_PARAMETER; }

    // We should try to create PKCS7 padding obj and pass it to bcrypt function but idk

    ULONG cbResult = 0; // Dont know if this shit can just be null tbh
    return BCryptEncrypt(
        hKey,
        (PUCHAR)pInput,
        InputSize,
        NULL,
        NULL,
        0,
        (PUCHAR)pOutput,
        OutputSize, 
        &cbResult,
        0
    );

}

NTSTATUS RequestCrypter::DecryptBuffer(PVOID pInput, ULONG InputSize, PVOID pOutput, ULONG OutputSize) {

    if (!hKey || !hAlg) { return STATUS_INVALID_HANDLE; }
    if (!pInput || !pOutput) { return STATUS_INVALID_PARAMETER; }

    if (InputSize % 16 != 0) { return STATUS_INVALID_PARAMETER; } // The buffer should be following PKCS7 standards and be padded from usermode - We remove padding after decryption

    ULONG cbResult = 0; // Still dont know if this shit can be null
    return BCryptDecrypt(
        hKey,
        (PUCHAR)pInput,
        InputSize,
        NULL,
        NULL,
        0,
        (PUCHAR)pOutput,
        OutputSize,
        &cbResult,
        0
    );

}