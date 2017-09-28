#include <kernel/pm/pm.h>
#include <kernel/panic.h>

#include <kernel/arch/x86/acpi/include/acpi.h>
#include <kernel/arch/x86/irq.h>

void pm_power_off(void) {
    if (ACPI_FAILURE(AcpiEnterSleepStatePrep(5))) {
        panic("ACPI shutdown failed.");
    }
    int_disable();

    AcpiEnterSleepState(ACPI_STATE_S5);
    panic("ACPI shutdown failed.");
}

void pm_reboot(void) {
    if (ACPI_FAILURE(AcpiReset())) {
        panic("ACPI reboot failed.");
    }
}
