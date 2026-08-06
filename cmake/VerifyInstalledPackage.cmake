if(NOT DEFINED SMARTGA_BUILD_DIR OR NOT DEFINED SMARTGA_SOURCE_DIR)
    message(FATAL_ERROR "SMARTGA_BUILD_DIR and SMARTGA_SOURCE_DIR are required")
endif()

set(install_prefix "${SMARTGA_BUILD_DIR}/consumer-prefix")
set(consumer_build_dir "${SMARTGA_BUILD_DIR}/consumer-build")
set(consumer_source_dir "${SMARTGA_SOURCE_DIR}/tests/consumer")

file(REMOVE_RECURSE "${install_prefix}" "${consumer_build_dir}")

set(build_config_args)
if(DEFINED SMARTGA_CONFIG AND NOT SMARTGA_CONFIG STREQUAL "")
    list(APPEND build_config_args --config "${SMARTGA_CONFIG}")
endif()

execute_process(
    COMMAND
        "${CMAKE_COMMAND}" --install "${SMARTGA_BUILD_DIR}"
        --prefix "${install_prefix}"
        ${build_config_args}
    RESULT_VARIABLE install_result
    COMMAND_ECHO STDOUT
)
if(NOT install_result EQUAL 0)
    message(FATAL_ERROR "SmartGA installation failed")
endif()

execute_process(
    COMMAND
        "${CMAKE_COMMAND}"
        -S "${consumer_source_dir}"
        -B "${consumer_build_dir}"
        "-DCMAKE_PREFIX_PATH=${install_prefix}"
    RESULT_VARIABLE configure_result
    COMMAND_ECHO STDOUT
)
if(NOT configure_result EQUAL 0)
    message(FATAL_ERROR "SmartGA consumer configuration failed")
endif()

execute_process(
    COMMAND
        "${CMAKE_COMMAND}" --build "${consumer_build_dir}"
        ${build_config_args}
    RESULT_VARIABLE build_result
    COMMAND_ECHO STDOUT
)
if(NOT build_result EQUAL 0)
    message(FATAL_ERROR "SmartGA consumer build failed")
endif()

set(consumer_executable_candidates
    "${consumer_build_dir}/smartga_consumer"
    "${consumer_build_dir}/smartga_consumer.exe"
)
if(DEFINED SMARTGA_CONFIG AND NOT SMARTGA_CONFIG STREQUAL "")
    list(PREPEND consumer_executable_candidates
        "${consumer_build_dir}/${SMARTGA_CONFIG}/smartga_consumer"
        "${consumer_build_dir}/${SMARTGA_CONFIG}/smartga_consumer.exe"
    )
endif()
list(APPEND consumer_executable_candidates
    "${consumer_build_dir}/Release/smartga_consumer"
    "${consumer_build_dir}/Release/smartga_consumer.exe"
)

unset(consumer_executable)
foreach(candidate IN LISTS consumer_executable_candidates)
    if(EXISTS "${candidate}")
        set(consumer_executable "${candidate}")
        break()
    endif()
endforeach()
if(NOT DEFINED consumer_executable)
    list(JOIN consumer_executable_candidates "\n  " searched_executables)
    message(FATAL_ERROR
        "SmartGA consumer executable was not found. Searched:\n  "
        "${searched_executables}")
endif()

execute_process(
    COMMAND "${consumer_executable}"
    RESULT_VARIABLE run_result
    COMMAND_ECHO STDOUT
)
if(NOT run_result EQUAL 0)
    message(FATAL_ERROR "SmartGA consumer execution failed")
endif()
