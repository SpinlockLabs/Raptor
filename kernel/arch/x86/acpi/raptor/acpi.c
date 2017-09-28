#include "../include/acpi.h"

#include <liblox/memory.h>
#include <liblox/sleep.h>

#include <kernel/arch/x86/paging.h>
#include <liblox/printf.h>
#include <kernel/arch/x86/io.h>
#include <kernel/spin.h>
#include <kernel/interupt.h>
#include <kernel/arch/x86/irq.h>

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

    if (Width == 8) {
        *Value = inb((uint16_t) Address);
    } else if (Width == 16) {
        *Value = ins((uint16_t) Address);
    } else if (Width == 32) {
        *Value = inl((uint16_t) Address);
    } else {
        return AE_ERROR;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsWritePort(
    ACPI_IO_ADDRESS Address,
    UINT32 Value,
    UINT32 Width) {

    if (Width == 8) {
        outb((uint16_t) Address, (uint8_t) Value);
    } else if (Width == 16) {
        outs((uint16_t) Address, (uint16_t) Value);
    } else if (Width == 32) {
        outl((uint16_t) Address, Value);
    } else {
        return AE_ERROR;
    }

    return AE_OK;
}

ACPI_STATUS AcpiOsSignal(UINT32 Function, void* Info) {
    return AE_OK;
}

ACPI_THREAD_ID AcpiOsGetThreadId(void) {
    return 1;
}

void AcpiOsWaitEventsComplete(void) {
    irq_wait();
}

ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer(void) {
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

ACPI_STATUS AcpiOsPhysicalTableOverride(
    ACPI_TABLE_HEADER* ExistingTable,
    ACPI_PHYSICAL_ADDRESS* NewAddress,
    UINT32* NewTableLength) {
    *NewAddress = 0;
    *NewTableLength = 0;
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
    return AE_OK;
}

ACPI_STATUS AcpiOsWriteMemory(
    ACPI_PHYSICAL_ADDRESS Address,
    UINT64 Value,
    UINT32 Width) {
    return AE_OK;
}

void* AcpiOsAllocate(ACPI_SIZE Size) {
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
    *Value = 0;
    return AE_OK;
}

ACPI_STATUS AcpiOsWritePciConfiguration(
    ACPI_PCI_ID* PciId,
    UINT32 Reg,
    UINT64 Value,
    UINT32 Width) {
    return AE_OK;
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
    uintptr_t phy = Physical;
    return (void*) paging_add_map(phy, Length);
}

void AcpiOsUnmapMemory(void* Where, ACPI_SIZE Length) {
}

void AcpiOsPrintf(const char* fmt, ...) {
#ifndef RAPTOR_ACPI_NO_DEBUG
    va_list args;
    va_start(args, fmt);
    char buffer[1024] = {0};
    vasprintf(buffer, fmt, args);
    va_end(args);
    puts(buffer);
#endif
}

void AcpiOsVprintf(const char* fmt, va_list vargs) {
#ifndef RAPTOR_ACPI_NO_DEBUG
    char buffer[1024] = {0};
    vasprintf(buffer, fmt, vargs);
    puts(buffer);
#endif
}

ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void* Context) {
    // TODO(kaendfinger): Make this use a real thread.
    Function(Context);
    return AE_OK;
}

void AcpiOsSleep(UINT64 ms) {
    sleep((ulong) ms);
}

void AcpiOsStall(UINT32 us) {
    // TODO(kaendfinger): Make this accurate.
    sleep(1);
}

UINT64 AcpiOsGetTimer(void) {
    return 0;
}

ACPI_STATUS AcpiOsCreateSemaphore(UINT32 Max, UINT32 Initial, ACPI_SEMAPHORE* OutHandle) {
    *OutHandle = NULL;
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
    *Handle = zalloc(sizeof(spin_lock_t));
    spin_init(*Handle);
    return AE_OK;
}

void AcpiOsDeleteLock(ACPI_SPINLOCK Handle) {
    free(Handle);
}

ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK Handle) {
    spin_lock(Handle);
    return 0;
}

void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags) {
    spin_unlock(Handle);
}

struct AcpiIrqHandler {
    ACPI_OSD_HANDLER Handler;
    void* Context;
    bool Registered;
};

static struct AcpiIrqHandler AcpiIrqHandlers[IRQ_CHAIN_SIZE];

int AcpiHandleIrq(cpu_registers_t* r) {
    uint32_t interrupt = r->int_no - 32;

    if (interrupt >= IRQ_CHAIN_SIZE) {
        return 0;
    }

    if (AcpiIrqHandlers[interrupt].Registered) {
        return AcpiIrqHandlers[interrupt].Handler(AcpiIrqHandlers[interrupt].Context);
    }
    return 0;
}

ACPI_STATUS AcpiOsInstallInterruptHandler(
    UINT32 InterruptNumber,
    ACPI_OSD_HANDLER Handler,
    void* Context) {
    AcpiIrqHandlers[InterruptNumber].Context = Context;
    AcpiIrqHandlers[InterruptNumber].Handler = Handler;
    if (!AcpiIrqHandlers[InterruptNumber].Registered) {
        AcpiIrqHandlers[InterruptNumber].Registered = true;
        irq_add_handler(InterruptNumber, AcpiHandleIrq);
    }
    printf(DEBUG "[ACPI] Installed IRQ handler for %d\n", InterruptNumber);
    return AE_OK;
}

ACPI_STATUS AcpiOsRemoveInterruptHandler(
    UINT32 InterruptNumber,
    ACPI_OSD_HANDLER Handler) {
    AcpiIrqHandlers[InterruptNumber].Registered = false;
    irq_remove_handler(InterruptNumber);
    return AE_OK;
}
