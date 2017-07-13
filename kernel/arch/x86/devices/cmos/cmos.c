#include <kernel/time.h>
#include <kernel/arch/x86/io.h>

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

#define CMOS_SECOND   0x0
#define CMOS_MINUTE   0x2
#define CMOS_HOUR     0x4
#define CMOS_DAY      0x7
#define CMOS_MONTH    0x8
#define CMOS_YEAR     0x9
#define CMOS_CENTURY  0x32
#define CMOS_STATUS   0x0A

#define from_bcd(val) (uint16_t)(((val / 16) * 10) + (val & 0x0f))

static uint8_t cmos_is_updating(void) {
    outb(CMOS_ADDRESS, CMOS_STATUS);
    return (uint8_t) (inb(CMOS_DATA) & 0x80);
}

static void cmos_dump(uint16_t* values) {
    while (cmos_is_updating());

    for (uint8_t idx = 0; idx < 128; ++idx) {
        outb(CMOS_ADDRESS, idx);
        values[idx] = inb(CMOS_DATA);
    }
}

void time_get(time_t* time) {
    uint16_t values[128];
    cmos_dump(values);

    time->second = from_bcd(values[CMOS_SECOND]);
    time->minute = from_bcd(values[CMOS_MINUTE]);
    time->hour = from_bcd(values[CMOS_HOUR]);
    time->day = from_bcd(values[CMOS_DAY]);
    time->month = from_bcd(values[CMOS_MONTH]);
    time->year = (uint16_t) (from_bcd(values[CMOS_CENTURY]) * 100 + from_bcd(values[CMOS_YEAR]));
}
