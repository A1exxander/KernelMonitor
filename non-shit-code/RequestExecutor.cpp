#include "RequestExecutor.h"


void RequestExecutor::DetectCheat(ULONG32 BSODMessage) {
    KeBugCheckEx(BSODMessage, NULL, NULL, NULL, NULL); 
}

NTSTATUS RequestExecutor::ReadCPUTemp(ULONG32& CPUTemp) {

    if (!DriverUtils::isIntelCPU()) { // Make sure to check because this shit will blow up an AMD cpu
        return STATUS_NOT_SUPPORTED;
    }

    constexpr ULONG32 IA32_THERM_STATUS = 0x19C;
    constexpr ULONG32 MSR_TEMPERATURE_TARGET = 0x1A2;

    ULONG64 ThermMSR = __readmsr(IA32_THERM_STATUS);
    ULONG64 TempTargetMSR = __readmsr(MSR_TEMPERATURE_TARGET);

    // Check that we can read further bits from the MSR via bit 31
    if (ThermMSR && !(1ULL << 31)) {
        return STATUS_FAIL_CHECK;
    }

    ULONG32 DigitalReadout = (ThermMSR >> 16) & 0x7F;
    ULONG32 TJMax = (TempTargetMSR >> 16) & 0xFF; // The max temp

    // Dont even ask why we have to do it this way
    CPUTemp = TJMax - DigitalReadout;
    return STATUS_SUCCESS;

}

NTSTATUS RequestExecutor::ReadGPUTemp(ULONG32& GPUTemp) { // Placeholder
    return STATUS_SUCCESS;
}

NTSTATUS RequestExecutor::ReadRAMUsage(ULONG64& UsedPhysicalMemoryMB) { // Main booby trap function

    ULONG64 UsedPhysicalMemoryB = 0;
    PHYSICAL_MEMORY_RANGE* MemoryRanges = MmGetPhysicalMemoryRanges();

    if (MemoryRanges == nullptr) {
        return STATUS_ACCESS_VIOLATION;
    }

    for (auto i = 0; MemoryRanges[i].BaseAddress.QuadPart != 0 || MemoryRanges[i].NumberOfBytes.QuadPart != 0; i++) { // I think the way this shit works is essentially last item in list will have quadpart == 0 

        PHYSICAL_ADDRESS Start = MemoryRanges[i].BaseAddress;
        PHYSICAL_ADDRESS End;
        End.QuadPart = Start.QuadPart + MemoryRanges[i].NumberOfBytes.QuadPart;

        while (Start.QuadPart < End.QuadPart) {

            PVOID VirtualAddress = MmGetVirtualForPhysical(Start);
            if (VirtualAddress != nullptr) {
                UsedPhysicalMemoryB += PAGE_SIZE;
            }
            Start.QuadPart += PAGE_SIZE;  // Move To The Next Page

        }
    }

    UsedPhysicalMemoryMB = UsedPhysicalMemoryB / (1024 * 1024); // Convert our bytes to MB
    ExFreePool(MemoryRanges); // I think we need to free this shit but it will probably BSOD us if we havent already 
    return STATUS_SUCCESS;

}

NTSTATUS RequestExecutor::ReadDiskInfo(const PUNICODE_STRING& DiskName, DISK_PERFORMANCE& DiskPerformanceReport) { // Only call this if you are brave

    HANDLE DiskHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IOStatusBlock;

    InitializeObjectAttributes(&ObjectAttributes, DiskName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

    // Open the disk file for reading attributes
    NTSTATUS DiskIOStatus = ZwCreateFile(&DiskHandle, FILE_READ_ATTRIBUTES, &ObjectAttributes, &IOStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN, FILE_NON_DIRECTORY_FILE, NULL, 0);

    if (!NT_SUCCESS(DiskIOStatus)) {
        return DiskIOStatus;
    }

    // Issue an IOCTL to get disk statistics
    DiskIOStatus = ZwDeviceIoControlFile(DiskHandle, NULL, NULL, NULL, &IOStatusBlock, IOCTL_DISK_PERFORMANCE, NULL, 0, &DiskPerformanceReport, sizeof(DISK_PERFORMANCE));
    
    ZwClose(DiskHandle);
    return DiskIOStatus;

}