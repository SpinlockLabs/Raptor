#include "block.h"

block_device_t* null_block_dev_create(char* name) {
    block_device_t* block = block_device_create(name);
    return block;
}
