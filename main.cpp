#pragma once
#pragma warning(disable:4100)
#include <ntifs.h>
#include "depend/kli.hpp"
#include "depend/structs.h"
#include "depend/sk.h"

extern "C" PVOID NTAPI PsGetProcessSectionBaseAddress(PEPROCESS Process);

ULONGLONG m_stored_dtb;

#include "req/base.h"
#include "req/cr3.h"
#include "req/read_write.h"

#include <ntddk.h>
#include "non-shit-code/IOCTLRequestExecutor.h"

constexpr auto DEVICE_NAME = L"\\Device\\SystemResourceMonitoringTechnologies"; // A kernel mode driver to monitor hardware components directly and do the occasional funny trick
constexpr auto SYMBOLIC_LINK_NAME = L"\\DosDevices\\SystemResourceMonitoringTechnologies";

void UnloadDriver(PDRIVER_OBJECT DriverObject);

NTSTATUS UnsupportedDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp);

NTSTATUS DeviceController(PDEVICE_OBJECT DeviceObject, PIRP Irp) {

    enum IOCTLCode { // May need to change if AC can read the codes

        DetectCheat = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x7FC, METHOD_BUFFERED, FILE_ANY_ACCESS),        // Funny easteregg
        ReadGPUTemp = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS),
        ReadCPUTemp = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS),
        ReadMemoryUsage = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS),   // Booby trap for Bastian H Suter
        ReadWriteMemory = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80C, METHOD_BUFFERED, FILE_ANY_ACCESS),
        ReadBaseAddress = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x810, METHOD_BUFFERED, FILE_ANY_ACCESS),
        DTB = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x814, METHOD_BUFFERED, FILE_ANY_ACCESS)                // Sure, whatever this means
    
    };

    PIO_STACK_LOCATION PIOStackLocation = IoGetCurrentIrpStackLocation(Irp);
    IOCTLCode RequestType = static_cast<IOCTLCode>(PIOStackLocation->Parameters.DeviceIoControl.IoControlCode);
    ULONG64 RequestBufferSize = PIOStackLocation->Parameters.DeviceIoControl.InputBufferLength; // Not needed for every request
    NTSTATUS RequestStatus = STATUS_SUCCESS;
    ULONG RequestResponseSize = 0;

    switch (RequestType) {

        case IOCTLCode::DetectCheat:

            IOCTLRequestExecutor::DetectCheat(0xDE7EC7ED);
            break;

        case IOCTLCode::ReadCPUTemp:

            ULONG32 CPUTemp;
            RequestStatus = IOCTLRequestExecutor::ReadCPUTemp(CPUTemp);
            RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, &CPUTemp, sizeof(CPUTemp)); // Create a function that can do this for us w any value
            Irp->IoStatus.Information = sizeof(CPUTemp);
            break;

        case IOCTLCode::ReadGPUTemp:

            //IOCTLRequestHandler::ReadGPUTemp();
            break;

        case IOCTLCode::ReadMemoryUsage:

            ULONG64 RAMUsageMB;
            RequestStatus = IOCTLRequestExecutor::ReadRAMUsage(RAMUsageMB);
            RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, &CPUTemp, sizeof(CPUTemp));
            Irp->IoStatus.Information = sizeof(RAMUsageMB);
            break;

        case IOCTLCode::ReadWriteMemory: // Fix all the trash below this please

            if (RequestBufferSize == sizeof(_rw)) {
                prw req = (prw)(Irp->AssociatedIrp.SystemBuffer);
                RequestStatus = ReadWriteHandler(req);
                RequestResponseSize = sizeof(_rw);
            }
            else {
                RequestStatus = STATUS_INFO_LENGTH_MISMATCH;
            }

            break;

        case IOCTLCode::ReadBaseAddress: 

            if (RequestBufferSize == sizeof(_ba)) {
                pba req = (pba)(Irp->AssociatedIrp.SystemBuffer);
                RequestStatus = get_image_base(req);
                RequestResponseSize = sizeof(_rw);
            }
            else {
                RequestStatus = STATUS_INFO_LENGTH_MISMATCH;
            }

            break;

        case IOCTLCode::DTB: {

            if (RequestBufferSize == sizeof sizeof(_dtb)) {
                dtbl req = (dtbl)(Irp->AssociatedIrp.SystemBuffer); 
                RequestStatus = FixDTB(req);
                RequestResponseSize = sizeof(_dtb);
            }
            else {
                RequestStatus = STATUS_INFO_LENGTH_MISMATCH;
            }

        }

        default:
            RequestStatus = STATUS_INVALID_DEVICE_REQUEST;

    }

    Irp->IoStatus.Status = RequestStatus;
    Irp->IoStatus.Information = RequestResponseSize;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return RequestStatus;

}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {

    UNICODE_STRING deviceName;
    UNICODE_STRING symbolicLinkName;
    RtlInitUnicodeString(&deviceName, DEVICE_NAME);
    RtlInitUnicodeString(&symbolicLinkName, SYMBOLIC_LINK_NAME);
    PDEVICE_OBJECT deviceObject;

    NTSTATUS driverInitilizationStatus = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &deviceObject);

    if (!NT_SUCCESS(driverInitilizationStatus)) {
        return driverInitilizationStatus;
    }

    driverInitilizationStatus = IoCreateSymbolicLink(&symbolicLinkName, &deviceName);
    if (!NT_SUCCESS(driverInitilizationStatus)) {
        IoDeleteDevice(deviceObject);
        return driverInitilizationStatus;
    }

    for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = UnsupportedDispatch;
    }

    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceController;
    DriverObject->DriverUnload = UnloadDriver;

    return STATUS_SUCCESS;

}

NTSTATUS UnsupportedDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp) {

    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Irp->IoStatus.Status;

}

void UnloadDriver(PDRIVER_OBJECT DriverObject) {

    UNICODE_STRING symbolicLinkName;
    RtlInitUnicodeString(&symbolicLinkName, SYMBOLIC_LINK_NAME);
    IoDeleteSymbolicLink(&symbolicLinkName);
    IoDeleteDevice(DriverObject->DeviceObject);

}

/*  TODO:
*       1. In my requests, make sure you are copying memory to the output buffer correctly
*       2. Make dedicated function using template for writing to output buffer called IOCTLRequestResponseGenerator or something like that
*       3. Remove illegal comments and rename everything to not be sus
*       4. Create IOCTLRequestHandler which wraps all of our switch cases and calls our RequestResponseGenerator
*       5. Encrypt / cipher our IOCTL parameters and output ( Do NOT encrypt the IOCTL code or we will probably get clapped )
*       6. Create an authentication mechanism for our usermode where if our requests dont have a specific parameter, we call readMemoryUsage
*       7. Update usermode interface to work with our new IOCTL codes
*       8. Refactor the bad code (everything outside this file and non-shit-code folder)
*/
