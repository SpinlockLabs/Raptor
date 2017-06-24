raptor_add_arch("x86" "kernel/arch/x86")

raptor_set_arch_ldflags(
  "x86"
  "-nostartfiles -ffreestanding -O2 -w -T ${RAPTOR_DIR}/kernel/arch/x86/linker.ld -m32"
)

raptor_set_arch_cflags(
  "x86"
  "-nostartfiles -std=gnu99 -ffreestanding -O2 -w -m32"
)
