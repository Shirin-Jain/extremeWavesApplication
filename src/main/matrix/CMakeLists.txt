add_executable(Matrix main.cpp)
target_link_libraries(Matrix mbed-os)
target_link_libraries(Matrix matrixtransfer)


set_target_properties(Matrix PROPERTIES
Matrix ${CMAKE_CURRENT_SOURCE_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}
        )

mbed_set_post_build(Matrix)

