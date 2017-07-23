#include "ubsan.h"

#include "../io.h"

void ubsan_debug(struct SourceLocation *location) {
    printf("[UBSAN] %s:%d:%dc - ", location->file_name,
            location->line, location->column);
}

void __ubsan_handle_add_overflow(struct OverflowData *data, unsigned long lhs,
                                 unsigned long rhs) {
    ubsan_debug(&data->location);
    printf("Overflow in add: %d %d\n", lhs, rhs);
}

void __ubsan_handle_sub_overflow(struct OverflowData *data, unsigned long lhs,
                                 unsigned long rhs) {
    ubsan_debug(&data->location);
    printf("Overflow in sub: %d %d\n", lhs, rhs);
}

void __ubsan_handle_mul_overflow(struct OverflowData *data, unsigned long lhs,
                                 unsigned long rhs) {
    ubsan_debug(&data->location);
    printf("Overflow in mul: %d %d\n", lhs, rhs);
}

void __ubsan_handle_divrem_overflow(struct OverflowData *data,
                                    unsigned long lhs, unsigned long rhs) {
    ubsan_debug(&data->location);
    printf("Overflow in divrem: %d %d\n", lhs, rhs);
}

void __ubsan_handle_negate_overflow(struct OverflowData *data,
                                    unsigned long old) {
    ubsan_debug(&data->location);
    printf("Overflow in negate: %d\n", old);
}

void __ubsan_handle_builtin_unreachable(struct UnreachableData *data) {
    ubsan_debug(&data->location);
    printf("Called __builtin_unreachable()\n");
}

void __ubsan_handle_out_of_bounds(struct OutOfBoundsData *data,
                                  unsigned long index) {
    ubsan_debug(&data->location);
    printf("Out of bounds array reference at %s[%d]\n",
            data->array_type->type_name, index);
}

void __ubsan_handle_shift_out_of_bounds(struct ShiftOutOfBoundsData *data,
                                        unsigned long lhs, unsigned long rhs) {
    ubsan_debug(&data->location);
    printf("Shift is out of bounds: %d %d\n", lhs, rhs);
}

#define IS_ALIGNED(a, b) (((a) & ((__typeof__(a))(b)-1)) == 0)

void __ubsan_handle_type_mismatch(struct TypeMismatchData *data,
                                  unsigned long ptr) {
    ubsan_debug(&data->location);
    if (data->alignment && !IS_ALIGNED(ptr, data->alignment)) {
        printf("Bad alignment in read at 0x%x (wanted %d)\n", ptr,
                data->alignment);
    } else {
        printf("Type mismatch in reference at 0x%x\n", ptr);
    }
}

void __ubsan_handle_type_mismatch_v1(struct TypeMismatchData *data,
                                     unsigned long ptr) {
    __ubsan_handle_type_mismatch(data, ptr);
}

void __ubsan_handle_nonnull_return(struct NonnullReturnData *data) {
    ubsan_debug(&data->location);
    printf("Expected function not to return null\n");
}

void __ubsan_handle_nonnull_arg(struct NonnullArgData *data) {
    ubsan_debug(&data->location);
    printf("Expected argument %d not to be null\n", data->arg_index);
}

void __ubsan_handle_vla_bound_not_positive(struct VLABoundData *data,
                                           unsigned long bound) {
    ubsan_debug(&data->location);
    printf("VLA bound not positive: %d\n", bound);
}

void __ubsan_handle_load_invalid_value(struct InvalidValueData *data,
				unsigned long val) {

    ubsan_debug(&data->location);
    printf("Invalid load of value at %d for the type %s: %d\n", val, data->type->type_name);
}
