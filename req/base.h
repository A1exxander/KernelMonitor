#pragma once

NTSTATUS get_image_base(pba f) {
	if (!f->process_id) {
		return STATUS_UNSUCCESSFUL;
	}
	PEPROCESS processs = NULL;
	KLI_FN(PsLookupProcessByProcessId)((HANDLE)f->process_id, &processs);
	if (!processs) {
		return STATUS_UNSUCCESSFUL;
	}
	ULONGLONG baseimg = (ULONGLONG)PsGetProcessSectionBaseAddress(processs);
	if (!baseimg) {
		return STATUS_UNSUCCESSFUL;
	}
	RtlCopyMemory(f->address, &baseimg, sizeof(baseimg));
	ObDereferenceObject(processs);
	return STATUS_SUCCESS;
}