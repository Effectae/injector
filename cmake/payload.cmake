set(${PROJECT_NAME}_PAYLOAD ${PROJECT_SOURCE_DIR}/payload)

add_executable(payload
    ${${PROJECT_NAME}_PAYLOAD}/${ANDROID_ABI}.c
    ${${PROJECT_NAME}_PAYLOAD}/common.c)

target_compile_options(payload PRIVATE
    -Os -nostdlib -ffreestanding
    -fno-builtin  -fno-stack-protector)

target_link_options(payload PRIVATE
    -nostdlib -static -Wl,--build-id=none
    -Wl,-T,${${PROJECT_NAME}_PAYLOAD}/payload.ld)
