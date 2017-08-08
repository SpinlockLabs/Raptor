#include "init.h"

#include <liblox/io.h>
#include <liblox/common.h>

#include <kernel/arch/x86/acpi/include/acpi.h>

#define _COMPONENT ACPI_EXAMPLE
ACPI_MODULE_NAME("acpica")

static void handle_acpi_notify(
    ACPI_HANDLE device,
    UINT32 value,
    void* context) {
    unused(device);
    unused(value);
    unused(context);
}

static ACPI_STATUS install_handlers(void) {
    ACPI_STATUS status;
    status = AcpiInstallNotifyHandler(
        ACPI_ROOT_OBJECT,
        ACPI_SYSTEM_NOTIFY,
        handle_acpi_notify,
        NULL
    );

    if (ACPI_FAILURE(status)) {
        ACPI_EXCEPTION((AE_INFO, status, "While installing notify handler"));
        return status;
    }

    return AE_OK;
}

void acpi_raptor_init(void) {
    printf(INFO "[ACPI] Initializing...\n");

    ACPI_STATUS status;

    status = AcpiInitializeSubsystem();
    if (ACPI_FAILURE(status)) {
        ACPI_EXCEPTION((AE_INFO, status, "While initializing ACPICA"));
        return;
    }

    status = AcpiInitializeTables(NULL, 0, FALSE);
    if (ACPI_FAILURE(status)) {
        ACPI_EXCEPTION((AE_INFO, status, "While initializing Table Manager"));
        return;
    }

    status = AcpiLoadTables();

    if (ACPI_FAILURE(status)) {
        ACPI_EXCEPTION((AE_INFO, status, "While loading tables"));
        return;
    }

    status = install_handlers();

    if (ACPI_FAILURE(status)) {
        ACPI_EXCEPTION((AE_INFO, status, "While installing handlers"));
        return;
    }

    status = AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);
    if (ACPI_FAILURE(status)) {
        ACPI_EXCEPTION((AE_INFO, status, "While enabling ACPICA"));
        return;
    }

    status = AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);

    if (ACPI_FAILURE(status)) {
        ACPI_EXCEPTION((AE_INFO, status, "While initializing ACPICA objects"));
        return;
    }

    printf(INFO "[ACPI] Initialized.\n");
}
