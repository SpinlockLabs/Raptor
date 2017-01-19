FUNCTION(LOAD_PROFILE ISA PLATFORM)
    # Obtain sources for the ISA
    FILE(GLOB ISA_SRCS RELATIVE ${CMAKE_SOURCE_DIR} "kernel/${ISA}/*.c" "kernel/${ISA}/*.[sS]" )
    FILE(GLOB PLATFORM_SRCS RELATIVE ${CMAKE_SOURCE_DIR} "kernel/${ISA}/${PLATFORM}/*.c" "kernel/${ISA}/${PLATFORM}/*.[sS]")

    # Load flags associated with ISA and Profile
    INCLUDE("${CMAKE_SOURCE_DIR}/kernel/${ISA}/flags.cmake")
    INCLUDE("${CMAKE_SOURCE_DIR}/kernel/${ISA}/${PLATFORM}/flags.cmake")

    # Now export our output variables
    SET(PLATFORM_LAYOUT "${CMAKE_SOURCE_DIR}/kernel/${ISA}/${PLATFORM}/layout.ld" PARENT_SCOPE)
    SET(ISA_SRCS ${ISA_SRCS} PARENT_SCOPE)
    SET(PLATFORM_SRCS ${PLATFORM_SRCS} PARENT_SCOPE)

    # And specific flags
    SET(ISA_C_FLAGS ${ISA_C_FLAGS} PARENT_SCOPE)
    SET(ISA_ASM_FLAGS ${ISA_ASM_FLAGS} PARENT_SCOPE)
ENDFUNCTION(LOAD_PROFILE)
