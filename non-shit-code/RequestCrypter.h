#pragma once
#include <ntddk.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib") // Sadly we got to use this crap since theres nothing else for kernel mode

enum CryptType {
    ENCRYPT,
    DECRYPT
};

class RequestCrypter {

private:

    UCHAR m_aesKey[16];
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;

public:

    RequestCrypter();
    RequestCrypter(const UCHAR* aesKey);
    ~RequestCrypter();
    void SetAESKey(const UCHAR* aesKey);
    NTSTATUS Initialize();
    NTSTATUS CryptBuffer(PVOID pInput, ULONG InputSize, PVOID pOutput, ULONG OutputSize, CryptType CryptType);
};