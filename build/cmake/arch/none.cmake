arch("none" "arch/generic")

message(
  WARNING
  "Building the fake 'none' architecture."
  "This is solely for development purposes."
)

add_definitions(-DARCH_NONE)
