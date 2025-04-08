add_executable(MatrixClient main.cpp)
target_link_libraries(MatrixClient mbed-os)
target_link_libraries(MatrixClient matrixtransfer)


set_target_properties(MatrixClient PROPERTIES
MatrixClient ${CMAKE_CURRENT_SOURCE_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}
        )

mbed_set_post_build(MatrixClient)

