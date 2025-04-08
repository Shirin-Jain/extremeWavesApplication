add_executable(MatrixServer main.cpp server.cpp server.h)
target_link_libraries(MatrixServer mbed-os matrixtransfer)

set_target_properties(MatrixServer PROPERTIES
MatrixServer ${CMAKE_CURRENT_SOURCE_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}
        )

mbed_set_post_build(MatrixServer)

