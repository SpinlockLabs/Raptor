#include "init.h"

#include <kernel/arch/x86/acpi/include/acpi.h>

void acpi_raptor_init(void) {
    AcpiInitializeSubsystem();
}
