
cmake_minimum_required(VERSION 3.22)
cmake_policy(VERSION 3.22)

find_package(Python3 REQUIRED)
execute_process(COMMAND ${Python3_EXECUTABLE} -m venv ${CMAKE_CURRENT_SOURCE_DIR}/venv)

set(ENV{VIRTUAL_ENV} "${CMAKE_CURRENT_SOURCE_DIR}//venv")
set(Python3_FIND_VIRTUALENV FIRST)
unset(Python3_EXECUTABLE)
find_package(Python3 COMPONENTS Interpreter Development)

execute_process(COMMAND
        ${Python3_EXECUTABLE} -m pip install -r ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt
        RESULT_VARIABLE PIP_RESULT_CODE)

if (NOT ${PIP_RESULT_CODE} EQUAL 0)
    message(FATAL_ERROR "Failed to install python requirements")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(UPLOAD_METHOD STM32CUBE)

set(MBED_TARGET NUCLEO_H723ZG)

set(MBED_APP_JSON_PATH mbed_app.json)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/UploadMethodConfig.cmake)

set(PYTHON_REQUIREMENTS_HAVE_BEEN_INSTALLED FALSE CACHE INTERNAL "Whether Python requirements have been installed")
if(NOT PYTHON_REQUIREMENTS_HAVE_BEEN_INSTALLED)
    execute_process(COMMAND
            ${Python3_EXECUTABLE} -m pip install -r ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os/tools/requirements.txt
            RESULT_VARIABLE PIP_RESULT_CODE)
    if(${PIP_RESULT_CODE} EQUAL 0)
        # Mark that we don't need to run pip again
        set(PYTHON_REQUIREMENTS_HAVE_BEEN_INSTALLED TRUE CACHE INTERNAL "" FORCE)
    endif()
endif()

add_compile_definitions(WITH_MBED_CE)

include(mbed-os/tools/cmake/mbed_toolchain_setup.cmake)

project(ExtremeWaves)

include(mbed-os/tools/cmake/mbed_project_setup.cmake)

add_subdirectory(mbed-os)
add_subdirectory(src/libs/matrixtransfer)

add_subdirectory(src/main/helloword)
add_subdirectory(src/main/matrix)
add_subdirectory(src/main/eightchipspi)
add_compile_options(-Wno-psabi)
