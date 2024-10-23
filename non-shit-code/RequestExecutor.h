#pragma once
#pragma warning(disable: 4100)
#include <ntddk.h>
#include <ntdddisk.h>
#include "DriverUtils.h"

namespace RequestExecutor {
	void DetectCheat(ULONG32 BSODMessage);
	NTSTATUS ReadCPUTemp(ULONG32& CPUTemp);
	NTSTATUS ReadGPUTemp(ULONG32& GPUTemp);
	NTSTATUS ReadRAMUsage(ULONG64& UsedPhysicalMemory);
	NTSTATUS ReadDiskInfo(const PUNICODE_STRING& DiskName, DISK_PERFORMANCE& DiskPerformanceReport);
}