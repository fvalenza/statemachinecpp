cmake_minimum_required(VERSION 3.14)

project(
    manager
    VERSION 0.1.0
    LANGUAGES CXX
)

# Compiler flags through an linked library.
include(cmake/base_compile_options.cmake)
include(cmake/documentation.cmake)

# sub-division of main sources into compile units can be called module or component
add_subdirectory(src/module1)

set(${PROJECT_NAME}_SOURCES
    src/manager.main.cpp
    src/states/states.cpp
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${lib_compiler_flags})
target_link_libraries(${PROJECT_NAME} PRIVATE libmodule1)

add_executable(mantest 
    src/mantest.main.cpp
)

target_include_directories(mantest
    PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
)

target_link_libraries(mantest PRIVATE ${lib_compiler_flags})
target_link_libraries(mantest PRIVATE libmodule1)
