add_library(serialib serialib.cpp serialib.h)
target_link_libraries(serialib)

target_include_directories(serialib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(serialib PRIVATE -Wno-unknown-pragmas)
