cmake_minimum_required(VERSION 3.20...3.31)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please use a separate build directory (e.g., build/)")
endif()

if(NOT DEFINED CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER "clang")
endif()
if(NOT DEFINED CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER "clang++")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(variant
    VERSION 1.0.0
    LANGUAGES CXX
)

add_executable(variant src/main.cpp)

target_compile_features(variant PRIVATE cxx_std_26)

target_include_directories(variant PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(variant PRIVATE -stdlib=libc++)
    target_link_options(variant PRIVATE -stdlib=libc++)
endif()

set(CXX_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
)

target_compile_options(variant PRIVATE ${CXX_WARNINGS})

target_compile_options(variant PRIVATE
    $<$<CONFIG:Debug>:-Werror>
    $<$<CONFIG:Debug>:-Wno-error=conversion>
    $<$<CONFIG:Debug>:-Wno-error=sign-conversion>
    $<$<CONFIG:Debug>:-Wno-error=old-style-cast>
)

target_compile_options(variant PRIVATE
    $<$<CONFIG:Release>:-O3>
)
