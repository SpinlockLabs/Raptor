#include <kernel/time.h>
#include <kernel/arch/x86/io.h>

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

#define CMOS_SECOND 0
#define CMOS_MINUTE 2
#define CMOS_HOUR   4
#define CMOS_DAY    7
#define CMOS_MONTH  8
#define CMOS_YEAR   9

#define from_bcd(val) (uint16_t)(((val / 16) * 10) + (val & 0x0f))

static void cmos_dump(uint16_t* values) {
    for (uint8_t idx = 0; idx < 128; ++idx)
    {
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
    time->year = from_bcd(values[CMOS_YEAR]);
}
