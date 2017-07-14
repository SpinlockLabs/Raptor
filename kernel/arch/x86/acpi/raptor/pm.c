#include <kernel/pm/pm.h>
#include <kernel/arch/x86/acpi/include/acpi.h>
#include <kernel/panic.h>

void pm_power_off(void) {
    AcpiEnterSleepStatePrep(5);
    asm volatile("cli");
    AcpiEnterSleepState(5);
    panic("pm_power_off x86");
}
