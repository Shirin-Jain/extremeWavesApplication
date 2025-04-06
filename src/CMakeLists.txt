add_executable(MatrixMain main.cpp)
target_link_libraries(MatrixMain mbed-os)

set_target_properties(MatrixMain PROPERTIES
        MatrixMain ${CMAKE_CURRENT_SOURCE_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}
        )

mbed_set_post_build(MatrixMain)

