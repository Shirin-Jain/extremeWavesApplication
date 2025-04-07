add_executable(HelloWorld main.cpp)
target_link_libraries(HelloWorld mbed-os)

set_target_properties(HelloWorld PROPERTIES
HelloWorld ${CMAKE_CURRENT_SOURCE_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}
        )

mbed_set_post_build(HelloWorld)

