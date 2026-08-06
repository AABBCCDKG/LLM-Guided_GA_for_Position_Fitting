cmake_minimum_required(VERSION 3.16)

project(SmartGA VERSION 1.0.0 LANGUAGES CXX)

option(SMARTGA_BUILD_TESTS "Build SmartGA unit tests" ON)
option(SMARTGA_ENABLE_OPENCV "Build optional OpenCV object detection support" ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

function(smartga_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /EHsc)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_library(smartga_core
    src/expression_function.cpp
    src/genetic_algorithm.cpp
)
add_library(SmartGA::core ALIAS smartga_core)
set_target_properties(smartga_core PROPERTIES EXPORT_NAME core)

target_include_directories(smartga_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(smartga_core PUBLIC cxx_std_20)
smartga_enable_warnings(smartga_core)

add_executable(smartga src/main.cpp)
target_link_libraries(smartga PRIVATE SmartGA::core)
smartga_enable_warnings(smartga)

if(SMARTGA_ENABLE_OPENCV)
    find_package(OpenCV QUIET COMPONENTS core imgproc imgcodecs)
    if(OpenCV_FOUND)
        add_library(smartga_detection src/object_detector.cpp)
        add_library(SmartGA::detection ALIAS smartga_detection)
        set_target_properties(smartga_detection PROPERTIES EXPORT_NAME detection)
        target_include_directories(smartga_detection
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
            PRIVATE
                ${OpenCV_INCLUDE_DIRS}
        )
        target_compile_features(smartga_detection PUBLIC cxx_std_20)
        target_link_libraries(smartga_detection PUBLIC ${OpenCV_LIBS})
        smartga_enable_warnings(smartga_detection)
        message(STATUS "OpenCV found: building SmartGA::detection")
    else()
        message(STATUS "OpenCV not found: building core GA without detection support")
    endif()
endif()

install(
    TARGETS smartga_core
    EXPORT SmartGATargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(
    TARGETS smartga
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(
    FILES
        include/expression_function.h
        include/genetic_algorithm.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

set(SMARTGA_PACKAGE_WITH_OPENCV OFF)
if(TARGET smartga_detection)
    set(SMARTGA_PACKAGE_WITH_OPENCV ON)
    install(
        TARGETS smartga_detection
        EXPORT SmartGATargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(
        FILES include/object_detector.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

set(SMARTGA_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/SmartGA")
configure_package_config_file(
    cmake/SmartGAConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/SmartGAConfig.cmake"
    INSTALL_DESTINATION "${SMARTGA_INSTALL_CMAKEDIR}"
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/SmartGAConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)
install(
    EXPORT SmartGATargets
    FILE SmartGATargets.cmake
    NAMESPACE SmartGA::
    DESTINATION "${SMARTGA_INSTALL_CMAKEDIR}"
)
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/SmartGAConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/SmartGAConfigVersion.cmake"
    DESTINATION "${SMARTGA_INSTALL_CMAKEDIR}"
)

if(SMARTGA_BUILD_TESTS)
    include(CTest)
    add_subdirectory(tests)
    add_test(
        NAME installed_package_consumer_test
        COMMAND
            ${CMAKE_COMMAND}
            "-DSMARTGA_BUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}"
            "-DSMARTGA_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DSMARTGA_CONFIG=$<CONFIG>"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/VerifyInstalledPackage.cmake"
    )
endif()
