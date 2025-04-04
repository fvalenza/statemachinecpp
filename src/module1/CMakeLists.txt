add_library(libmodule1 module1.cpp)

target_include_directories(libmodule1
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        # $<INSTALL_INTERFACE:include>
)

