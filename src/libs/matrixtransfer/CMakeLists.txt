add_library(matrixtransfer matrixtransfer.cpp matrixtransfer.h)

target_include_directories(matrixtransfer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(matrixtransfer PRIVATE -Wno-unknown-pragmas)
