cmake_minimum_required(VERSION 3.10)
project(MyLZMA C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(inc)

set(LIB_SRCS
    src/lzma.c
    src/lzss_parser.c
    src/lzss_decoder.c
    src/search_window.c
    src/models.c
    src/range_coder.c
    src/range_decoder.c
)

add_executable(notlzma
    src/main.c
    ${LIB_SRCS}
)

add_executable(test_lzma_roundtrip
    tests/test_lzma_roundtrip.c
    ${LIB_SRCS}
)

if (UNIX)
    target_link_libraries(test_lzma_roundtrip m)
endif()

target_compile_definitions(notlzma PRIVATE _FILE_OFFSET_BITS=64)
target_compile_definitions(test_lzma_roundtrip PRIVATE _FILE_OFFSET_BITS=64)
