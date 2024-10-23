#pragma once
#include <ntddk.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib") // Sadly we got to use this crap since theres nothing else for kernel mode

class RequestCrypter {

private:

    UCHAR m_aesKey[16];
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;
    static constexpr ULONG AES_BLOCK_SIZE = 16;
    NTSTATUS RemovePadding(PVOID pBuffer, ULONG BufferSize, ULONG& UnpaddedBufferSize);

public:

    RequestCrypter();
    RequestCrypter(const UCHAR* aesKey);
    ~RequestCrypter();
    void SetAESKey(const UCHAR* aesKey);
    NTSTATUS Initialize();
    NTSTATUS EncryptBuffer(PVOID pInput, ULONG InputSize, PVOID pOutput, ULONG& OutputSize);
    NTSTATUS DecryptBuffer(PVOID pInput, ULONG InputSize, PVOID pOutput, ULONG& OutputSize);

};