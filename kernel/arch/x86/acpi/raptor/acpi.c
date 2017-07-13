#include "acpi.h"

#include "../include/acpi.h"

#include <liblox/memory.h>
#include <liblox/sleep.h>

#include <kernel/arch/x86/paging.h>

ACPI_STATUS AcpiOsInitialize(void) {
    return AE_OK;
}

ACPI_STATUS AcpiOsTerminate(void) {
    return AE_OK;
}

ACPI_PHYSICAL_ADDRESS AcpiGetRootPointer(void) {
    ACPI_PHYSICAL_ADDRESS ret = 0;
    AcpiFindRootPointer(&ret);
    return ret;
}

ACPI_STATUS AcpiOsPredefinedOverride(
    const ACPI_PREDEFINED_NAMES* PredefinedObject,
    ACPI_STRING* NewValue
) {
    *NewValue = NULL;
    return AE_OK;
}

ACPI_STATUS AcpiOsTableOverride(
    ACPI_TABLE_HEADER* ExistingTable,
    ACPI_TABLE_HEADER** NewTable) {
    *NewTable = NULL;
    return AE_OK;
}

ACPI_STATUS AcpiOsGetPhysicalAddress(void* Logical, ACPI_PHYSICAL_ADDRESS* Physical) {
    *Physical = paging_get_physical_address((uintptr_t) Logical);
    return AE_OK;
}

void* AcpiOsAllocate(ACPI_SIZE Size) {
    return malloc(Size);
}

void AcpiOsFree(void* Memory) {
    free(Memory);
}

BOOLEAN AcpiOsReadable(void* Memory, ACPI_SIZE Length) {
    page_t* page = paging_get_page(
        (uintptr_t) Memory,
        0,
        paging_get_directory());

    return page != NULL;
}

BOOLEAN AcpiOsWritable(void* Memory, ACPI_SIZE Length) {
    page_t* page = paging_get_page(
        (uintptr_t) Memory,
        0,
        paging_get_directory());

    return page != NULL && page->rw == 1;
}

void* AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS Physical, ACPI_SIZE Length) {
    return (void*) paging_add_map(Physical, Length);
}

void AcpiOsUnmapMemory(void* Where, ACPI_SIZE Length) {
}

void AcpiOsPrintf(const char* fmt, ...) {
}

void AcpiOsVprintf(const char* fmt, va_list vargs) {
}

ACPI_STATUS AcpiOsPurgeCache(void** Cache) {
    return AE_OK;
}

ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void* Context) {
    // TODO(kaendfinger): Make this use a real thread.
    Function(Context);
    return AE_OK;
}

void AcpiOsSleep(UINT64 ms) {
    sleep(ms);
}

void AcpiOsStall(UINT32 us) {
    // TODO(kaendfinger): Make this accurate.
    sleep(1);
}

ACPI_STATUS AcpiOsCreateSemaphore(UINT32 Max, UINT32 Initial, ACPI_SEMAPHORE* OutHandle) {
    return AE_OK;
}

ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle) {
    return AE_OK;
}

ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units) {
    return AE_OK;
}

ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units, UINT16 Timeout) {
    return AE_OK;
}

ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK* Handle) {
    return AE_OK;
}

void AcpiOsDeleteLock(ACPI_SPINLOCK Handle) {}

ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK Handle) {
    return 0;
}

void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags) {}

ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER Handler, void* Context) {
    return AE_OK;
}

ACPI_STATUS AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER Handler) {
    return AE_OK;
}
