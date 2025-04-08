add_executable(MatrixClient main.cpp client.cpp client.h)
target_link_libraries(MatrixClient matrixtransfer serialib)

set_target_properties(MatrixClient PROPERTIES MatrixClient ${CMAKE_CURRENT_SOURCE_DIR} RUNTIME_OUTPUT_DIRECTORY_DEVELOP ${CMAKE_CURRENT_SOURCE_DIR})
