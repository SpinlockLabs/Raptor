.section .text
.global _start
.type _start, @function
_start:
  call __do_main
