#include <kernel/pm/pm.h>
#include <kernel/panic.h>

#include <kernel/arch/x86/acpi/include/acpi.h>
#include <kernel/arch/x86/irq.h>

void pm_power_off(void) {
    if (AcpiEnterSleepStatePrep(5) != AE_OK) {
        panic("ACPI shutdown failed.");
    }
    int_disable();
    AcpiEnterSleepState(5);
    panic("pm_power_off x86");
}
