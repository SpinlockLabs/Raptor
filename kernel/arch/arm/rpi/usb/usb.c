#include "usb.h"

#include <liblox/io.h>

#include <uspi.h>
#include <uspios.h>

#include <liblox/string.h>
#include <liblox/printf.h>
#include <kernel/arch/arm/rpi/irq.h>
#include <kernel/arch/arm/rpi/delay.h>

#include "../mailbox.h"

void UspiLogWrite(const char* pSource,
                  unsigned Severity,
                  const char* pMessage, ...) {
    va_list list;
    va_start(list, pMessage);
    char msg[128] = {0};
    vasprintf(msg, pMessage, list);
    printf(DEBUG "[USB] %s\n", msg);
    va_end(list);
}

void UspiMsDelay(unsigned nMilliSeconds) {
    delay(500 * nMilliSeconds);
}

void UspiUsDelay(unsigned nMicroSeconds) {
    delay(50 * nMicroSeconds);
}

unsigned UspiStartKernelTimer(unsigned nHzDelay,
                              TKernelTimerHandler* pHandler,
                              void* pParam, void* pContext) {
    printf("Start Timer delay = %d\n", nHzDelay);
    return 0;
}

void UspiCancelKernelTimer(unsigned hTimer) {
    printf("Cancel Timer %d\n", hTimer);
}

void UspiConnectInterrupt(unsigned nIRQ, TInterruptHandler* pHandler, void* pParam) {
    irq_set_handler(
        nIRQ,
        pHandler,
        pParam
    );
}

int UspiSetPowerStateOn(unsigned nDeviceId) {
    bcm_property_power_t power = {0};
    power.device_id = nDeviceId;
    power.state = BCM_POWER_STATE_ON | BCM_POWER_STATE_WAIT;

    bool result = bcm_get_property_tag(
        BCM_TAG_SET_POWER_STATE,
        &power,
        sizeof(bcm_property_power_t),
        8
    );

    if (!result) {
        return 0;
    }

    if ((power.state & BCM_POWER_STATE_NO_DEVICE) ||
        !(power.state & BCM_POWER_STATE_ON)) {
        return 0;
    }

    return 1;
}

int UspiGetMACAddress(unsigned char* Buffer) {
    bcm_property_mac_t mac = {0};
    if (!bcm_get_property_tag(
        BCM_TAG_GET_MAC_ADDRESS,
        &mac,
        sizeof(bcm_property_mac_t),
        0
    )) {
        return 0;
    }

    memcpy(Buffer, mac.address, 6);

    return 1;
}

void usb_rpi_init(void) {
    printf(DEBUG "USB initializing...\n");

    if (!USPiInitialize()) {
        return;
    }

    if (USPiEthernetAvailable()) {
        printf(DEBUG "Ethernet is available.\n");
    }

    if (USPiMouseAvailable()) {
        printf(DEBUG "Mouse is available.\n");
    }

    if (USPiKeyboardAvailable()) {
        printf(DEBUG "Keyboard is available.\n");
    }

    if (USPiMassStorageDeviceAvailable()) {
        printf(DEBUG "Mass Storage is available.\n");
    }

    printf(DEBUG "USB initialized.\n");
}
