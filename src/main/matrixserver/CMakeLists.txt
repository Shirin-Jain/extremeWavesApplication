add_executable(MatrixServer main.cpp)
target_link_libraries(MatrixServer mbed-os)
target_link_libraries(MatrixServer matrixtransfer)


set_target_properties(MatrixServer PROPERTIES
MatrixServer ${CMAKE_CURRENT_SOURCE_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}
        )

mbed_set_post_build(MatrixServer)

