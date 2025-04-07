add_executable(eightchipspi main.cpp)
target_link_libraries(eightchipspi mbed-os)

set_target_properties(eightchipspi PROPERTIES
eightchipspi ${CMAKE_CURRENT_SOURCE_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}
        )

mbed_set_post_build(eightchipspi)

