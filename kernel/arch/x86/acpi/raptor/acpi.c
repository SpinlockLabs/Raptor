#include "acpi.h"

#include "../include/acpi.h"
#include "../rprintf/printf.h"

#include <liblox/memory.h>
#include <liblox/sleep.h>

#include <kernel/arch/x86/paging.h>

ACPI_STATUS AcpiOsInitialize(void) {
    return AE_OK;
}

ACPI_STATUS AcpiOsTerminate(void) {
    return AE_OK;
}

ACPI_STATUS AcpiOsEnterSleep(
    UINT8 SleepState,
    UINT32 RegaValue,
    UINT32 RegbValue) {
    return AE_OK;
}

ACPI_STATUS AcpiOsReadPort(
    ACPI_IO_ADDRESS Address,
    UINT32* Value,
    UINT32 Width) {
    return AE_OK;
}

ACPI_STATUS AcpiOsWritePort(
    ACPI_IO_ADDRESS Address,
    UINT32 Value,
    UINT32 Width) {
    return AE_OK;
}

ACPI_STATUS AcpiOsSignal(UINT32 Function, void* Info) {
    return AE_OK;
}

ACPI_THREAD_ID AcpiOsGetThreadId(void) {
    return 0;
}

void AcpiOsWaitEventsComplete(void) {
}

ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer(void) {
    printf("[ACPI] Get root pointer.\n");

    ACPI_PHYSICAL_ADDRESS ret = 0;
    AcpiFindRootPointer(&ret);
    return ret;
}

void* AcpiOsAcquireObject(ACPI_CACHE_T* Cache) {
    return 0;
}

ACPI_STATUS AcpiOsReleaseObject(ACPI_CACHE_T* Cache, void* Object) {
    return AE_OK;
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

ACPI_STATUS AcpiOsPhysicalTableOverride(
    ACPI_TABLE_HEADER* ExistingTable,
    ACPI_PHYSICAL_ADDRESS* NewAddress,
    UINT32* NewTableLength) {
    return AE_OK;
}

ACPI_STATUS AcpiOsGetPhysicalAddress(void* Logical, ACPI_PHYSICAL_ADDRESS* Physical) {
    *Physical = paging_get_physical_address((uintptr_t) Logical);
    return AE_OK;
}

ACPI_STATUS AcpiOsReadMemory(
    ACPI_PHYSICAL_ADDRESS Address,
    UINT64* Value,
    UINT32 Width) {
    printf("[ACPI] Read memory 0x%x\n", Address);
    return AE_OK;
}

ACPI_STATUS AcpiOsWriteMemory(
    ACPI_PHYSICAL_ADDRESS Address,
    UINT64 Value,
    UINT32 Width) {
    printf("[ACPI] Write memory 0x%x\n", Address);
    return AE_OK;
}

void* AcpiOsAllocate(ACPI_SIZE Size) {
    printf("[ACPI] Allocate %d\n", Size);
    return malloc(Size);
}

void AcpiOsFree(void* Memory) {
    free(Memory);
}

ACPI_STATUS AcpiOsReadPciConfiguration(
    ACPI_PCI_ID* PciId,
    UINT32 Reg,
    UINT64* Value,
    UINT32 Width) {
    printf("[ACPI] Read PCI configuration\n");
    return AE_OK;
}

ACPI_STATUS AcpiOsWritePciConfiguration(
    ACPI_PCI_ID* PciId,
    UINT32 Reg,
    UINT64 Value,
    UINT32 Width) {
    printf("[ACPI] Write PCI configuration\n");
    return AE_OK;
}

BOOLEAN AcpiOsReadable(void* Memory, ACPI_SIZE Length) {
    printf("[ACPI] Is readable 0x%x\n", Memory);

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
    printf("[ACPI] Map Memory 0x%x (len = %d)\n", Physical, Length);
    return (void*) paging_add_map((uintptr_t) Physical, Length);
}

void AcpiOsUnmapMemory(void* Where, ACPI_SIZE Length) {
    printf("[ACPI] Unmap Memory 0x%x (len = %d)\n", Where, Length);
}

void AcpiOsPrintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buffer[256] = {0};
    mini_vsnprintf(buffer, 256, fmt, args);
    va_end(args);
    puts(buffer);
}

void AcpiOsVprintf(const char* fmt, va_list vargs) {
    char buffer[256] = {0};
    mini_vsnprintf(buffer, 256, fmt, vargs);
    puts(buffer);
}

ACPI_STATUS AcpiOsCreateCache(char* CacheName, UINT16 ObjectSize,
                              UINT16 MaxDepth, ACPI_CACHE_T** ReturnCache) {
    return AE_OK;
}

ACPI_STATUS AcpiOsDeleteCache(ACPI_CACHE_T* Cache) {
    return AE_OK;
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

UINT64 AcpiOsGetTimer(void) {
    return 0;
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

ACPI_STATUS AcpiOsInstallInterruptHandler(
    UINT32 InterruptNumber,
    ACPI_OSD_HANDLER Handler,
    void* Context) {
    printf("[ACPI] Install interrupt %d\n", InterruptNumber);
    return AE_OK;
}

ACPI_STATUS AcpiOsRemoveInterruptHandler(
    UINT32 InterruptNumber,
    ACPI_OSD_HANDLER Handler) {
    printf("[ACPI] Remove interrupt %d\n", InterruptNumber);
    return AE_OK;
}
