cmake_minimum_required(VERSION 3.30)

project(Injector LANGUAGES C
    VERSION "0.0.1")

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_EXTENSIONS OFF)

add_executable(${PROJECT_NAME})

file(GLOB_RECURSE ${PROJECT_NAME}_SRC CONFIGURE_DEPENDS 
    ${PROJECT_SOURCE_DIR}/src/*.c)

target_sources(${PROJECT_NAME}
    PRIVATE
    ${${PROJECT_NAME}_SRC}
    ${PROJECT_SOURCE_DIR}/src/spe/${ANDROID_ABI}.c)

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -D_POSIX_C_SOURCE=200809L")

include(cmake/payload.cmake)
include(cmake/find_objcopy.cmake)

add_custom_command(
    OUTPUT payload.bin
    COMMAND ${OBJCOPY_EXE} -j .text -O binary 
        $<TARGET_FILE:payload>
        payload.bin
    DEPENDS payload
    COMMENT "Extracting payload.bin form payload")

set_property(
    SOURCE ${PROJECT_SOURCE_DIR}/src/payload.c
    APPEND
    PROPERTY OBJECT_DEPENDS payload.bin)

set_source_files_properties(${PROJECT_SOURCE_DIR}/src/payload.c
    PROPERTIES COMPILE_OPTIONS "--embed-dir=${CMAKE_CURRENT_BINARY_DIR}")

set_source_files_properties(${PROJEC_SOURCE_DIR}/src/main.c
    PROPERTIES COMPILE_DEFINITIONS "INJECTOR_VERSION=\"${PROJECT_VERSION}\"")

option(BUILD_EXAMPLE_LIBRARY "Build example library" ON)
if(BUILD_EXAMPLE_LIBRARY)
    add_subdirectory(example_lib)
endif()
