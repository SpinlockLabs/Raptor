#pragma once

enum {
#ifdef PI_1
    BOARD_PERIPHERALS_BASE = 0x20000000,
#else
    BOARD_PERIPHERALS_BASE = 0x3f000000,
#endif
    BOARD_PERIPHERALS_SIZE = 0x01000000,
    BOARD_UART0_BASE = (BOARD_PERIPHERALS_BASE + 0x00201000),
    BOARD_TIMER_CTL = (BOARD_PERIPHERALS_BASE + 0xB408),
    BOARD_TIMER_CNT = (BOARD_PERIPHERALS_BASE + 0xB420),
    BOARD_MAILBOX_BASE = (BOARD_PERIPHERALS_BASE + 0xB880),

    BOARD_SYS_TIMER_BASE = (BOARD_PERIPHERALS_BASE + 0x3000),
    BOARD_SYS_TIMER_CTL = (BOARD_SYS_TIMER_BASE + 0x00),
    BOARD_SYS_TIMER_CLO = (BOARD_PERIPHERALS_BASE + 0x04),
    BOARD_SYS_TIMER_CHI = (BOARD_PERIPHERALS_BASE + 0x08),
    BOARD_SYS_TIMER_C0 = (BOARD_PERIPHERALS_BASE + 0x0C),
    BOARD_SYS_TIMER_C1 = (BOARD_PERIPHERALS_BASE + 0x10),
    BOARD_SYS_TIMER_C2 = (BOARD_PERIPHERALS_BASE + 0x14),
    BOARD_SYS_TIMER_C3 = (BOARD_PERIPHERALS_BASE + 0x18),

    BOARD_EXCEPTION_TABLE_BASE = 0x00000004,

    BOARD_IC_BASE = (BOARD_PERIPHERALS_BASE + 0xB000),
    BOARD_IC_IRQ_BASIC_PENDING = (BOARD_IC_BASE + 0x200),
    BOARD_IC_IRQ_PENDING_1 = (BOARD_IC_BASE + 0x204),
    BOARD_IC_IRQ_PENDING_2 = (BOARD_IC_BASE + 0x208),
    BOARD_IC_FIQ_CTL = (BOARD_IC_BASE + 0x20C),
    BOARD_IC_ENABLE_IRQS_1 = (BOARD_IC_BASE + 0x210),
    BOARD_IC_ENABLE_IRQS_2 = (BOARD_IC_BASE + 0x214),
    BOARD_IC_ENABLE_BASIC_IRQS = (BOARD_IC_BASE + 0x218),
    BOARD_IC_DISABLE_IRQS_1 = (BOARD_IC_BASE + 0x21C),
    BOARD_IC_DISABLE_IRQS_2 = (BOARD_IC_BASE + 0x220),
    BOARD_IC_DISABLE_BASIC_IRQS = (BOARD_IC_BASE + 0x224)
};

enum {
#if 0
    PI_FIRMWARE_BASE = 0x40000000,
#else
    PI_FIRMWARE_BASE = 0xC0000000,
#endif
};

#define BOARD_BUS_ADDRESS(phys) (((phys) & ~0xC0000000) | PI_FIRMWARE_BASE)
