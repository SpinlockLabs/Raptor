arch("user" "arch/user")

add_definitions(
  -DARCH_USER
)

target_link_libraries(kernel dl)
