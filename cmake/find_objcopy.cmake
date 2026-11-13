find_program(OBJCOPY_EXE
    NAMES llvm-objcopy objcopy
    HINTS
    ${CMAKE_C_COMPILER}../..
    ${ANDROID_TOOLCHAIN_ROOT}/bin)

if(NOT OBJCOPY_EXE)
    message(FATAL_ERROR "objcopy not found.")
endif()

