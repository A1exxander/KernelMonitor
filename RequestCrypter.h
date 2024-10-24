#pragma once
#include <ntddk.h>
#include <bcrypt.h>
#include "Globals.h"
#pragma comment(lib, "bcrypt.lib") // Sadly we got to use this crap since theres nothing else for kernel mode

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
    NTSTATUS EncryptBuffer(PVOID pInput, ULONG InputSize, PUCHAR pOutput, ULONG OutputSize);
    NTSTATUS DecryptBuffer(PUCHAR pInput, ULONG InputSize, PVOID pOutput, ULONG& OutputSize);

};