cmake_minimum_required(VERSION 3.16)

project(SmartGAConsumer LANGUAGES CXX)

find_package(SmartGA 1 CONFIG REQUIRED)

if(NOT TARGET SmartGA::core)
    message(FATAL_ERROR "SmartGA::core is missing from the installed package")
endif()
if(SmartGA_WITH_OPENCV AND NOT TARGET SmartGA::detection)
    message(FATAL_ERROR "OpenCV-enabled package is missing SmartGA::detection")
endif()
if(NOT SmartGA_WITH_OPENCV AND TARGET SmartGA::detection)
    message(FATAL_ERROR "Core-only package unexpectedly exports SmartGA::detection")
endif()

add_executable(smartga_consumer main.cpp)
target_link_libraries(smartga_consumer PRIVATE SmartGA::core)
