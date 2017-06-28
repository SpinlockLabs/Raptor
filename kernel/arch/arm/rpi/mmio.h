#include <liblox/common.h>
#include <stdint.h>

#define mmio_u32_op(x) *((volatile uint32_t*) (x))

uint32_t mmio_u32_read(uint32_t reg);
void mmio_u32_write(uint32_t reg, uint32_t data);
