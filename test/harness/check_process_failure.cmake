if(NOT DEFINED PROBE_EXECUTABLE OR "${PROBE_EXECUTABLE}" STREQUAL "")
    message(FATAL_ERROR "PROBE_EXECUTABLE must name the probe executable")
endif()
if(NOT DEFINED PROBE_SOURCE_FILE OR "${PROBE_SOURCE_FILE}" STREQUAL "")
    message(FATAL_ERROR "PROBE_SOURCE_FILE must name the probe source file")
endif()
if(NOT DEFINED PROBE_DETACHED_EXPRESSION
   OR "${PROBE_DETACHED_EXPRESSION}" STREQUAL "")
    message(FATAL_ERROR
        "PROBE_DETACHED_EXPRESSION must name the detached expression")
endif()
if(NOT DEFINED PROBE_DETACHED_FAILURE_COUNT
   OR "${PROBE_DETACHED_FAILURE_COUNT}" STREQUAL ""
   OR NOT "${PROBE_DETACHED_FAILURE_COUNT}" MATCHES "^[0-9]+$")
    message(FATAL_ERROR
        "PROBE_DETACHED_FAILURE_COUNT must provide a non-negative integer")
endif()
if(NOT DEFINED PROBE_ENCLOSING_SUITE_NAME
   OR "${PROBE_ENCLOSING_SUITE_NAME}" STREQUAL "")
    message(FATAL_ERROR
        "PROBE_ENCLOSING_SUITE_NAME must name the enclosing suite")
endif()
if(NOT DEFINED PROBE_TIMEOUT)
    set(PROBE_TIMEOUT 30)
endif()
if(NOT PROBE_TIMEOUT MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR "PROBE_TIMEOUT must be a positive integer")
endif()
execute_process(
    COMMAND "${PROBE_EXECUTABLE}"
    RESULT_VARIABLE probe_status
    OUTPUT_VARIABLE probe_stdout
    ERROR_VARIABLE probe_error
    TIMEOUT ${PROBE_TIMEOUT})

function(fail_probe reason)
    message(FATAL_ERROR
        "${reason}\n"
        "stdout:\n${probe_stdout}\n"
        "stderr:\n${probe_error}")
endfunction()

if("${probe_status}" MATCHES "[Tt]imeout")
    fail_probe("Process failure probe timed out")
endif()

if(NOT "${probe_status}" MATCHES "^[0-9]+$")
    fail_probe("Process failure probe did not exit normally: ${probe_status}")
endif()

if(NOT "${probe_status}" STREQUAL "1")
    fail_probe("Process failure probe returned ${probe_status} instead of 1")
endif()

string(STRIP "${probe_stdout}" stripped_probe_stdout)
string(REPLACE "\r\n" "\n" normalized_probe_stdout
    "${stripped_probe_stdout}")
string(REPLACE "\\" "/" normalized_probe_stdout
    "${normalized_probe_stdout}")
set(detached_failure_count 0)
set(diagnostic_line_numbers)
get_filename_component(probe_source_name "${PROBE_SOURCE_FILE}" NAME)
string(CONCAT failure_prefix "[FAIL] ")
string(LENGTH "${failure_prefix}" failure_prefix_length)
string(CONCAT enclosed_failure_prefix
    "[FAIL] ${PROBE_ENCLOSING_SUITE_NAME}:")
string(CONCAT expression_prefix ": ${PROBE_DETACHED_EXPRESSION}")
string(LENGTH "${expression_prefix}" expression_prefix_length)
string(CONCAT source_name_suffix "/${probe_source_name}:")
string(CONCAT source_name_only_suffix "${probe_source_name}:")
string(LENGTH "${source_name_suffix}" source_name_suffix_length)
string(LENGTH "${source_name_only_suffix}" source_name_only_suffix_length)
while(NOT "${normalized_probe_stdout}" STREQUAL "")
    string(FIND "${normalized_probe_stdout}" "\n" newline_position)
    if(newline_position EQUAL -1)
        set(probe_stdout_line "${normalized_probe_stdout}")
        set(normalized_probe_stdout "")
    else()
        string(SUBSTRING "${normalized_probe_stdout}" 0 ${newline_position}
            probe_stdout_line)
        math(EXPR next_line_position "${newline_position} + 1")
        string(SUBSTRING "${normalized_probe_stdout}"
            ${next_line_position} -1 normalized_probe_stdout)
    endif()
    string(STRIP "${probe_stdout_line}" probe_stdout_line)
    if("${probe_stdout_line}" STREQUAL "")
        continue()
    endif()
    string(FIND "${probe_stdout_line}" "${enclosed_failure_prefix}"
        enclosed_failure_position)
    if(enclosed_failure_position EQUAL 0)
        fail_probe("Detached diagnostic used the enclosing suite name")
    endif()
    string(LENGTH "${probe_stdout_line}" probe_stdout_line_length)
    if(probe_stdout_line_length LESS failure_prefix_length)
        fail_probe("Process failure probe emitted unexpected stdout")
    endif()
    string(SUBSTRING "${probe_stdout_line}" 0 ${failure_prefix_length}
        actual_failure_prefix)
    if(NOT "${actual_failure_prefix}" STREQUAL "${failure_prefix}")
        fail_probe("Process failure probe emitted unexpected stdout")
    endif()
    if(probe_stdout_line_length LESS expression_prefix_length)
        fail_probe("Process failure probe emitted unexpected stdout")
    endif()
    math(EXPR expression_prefix_start
        "${probe_stdout_line_length} - ${expression_prefix_length}")
    string(SUBSTRING "${probe_stdout_line}" ${expression_prefix_start} -1
        actual_expression_prefix)
    if(NOT "${actual_expression_prefix}" STREQUAL "${expression_prefix}")
        fail_probe("Process failure probe emitted an unexpected expression")
    endif()
    math(EXPR diagnostic_metadata_length
        "${expression_prefix_start} - ${failure_prefix_length}")
    string(SUBSTRING "${probe_stdout_line}" ${failure_prefix_length}
        ${diagnostic_metadata_length} diagnostic_metadata)
    string(FIND "${diagnostic_metadata}" "${source_name_suffix}"
        source_name_position)
    if(source_name_position EQUAL -1)
        string(FIND "${diagnostic_metadata}" "${source_name_only_suffix}"
            source_name_position)
        if(NOT source_name_position EQUAL 0)
            fail_probe("Process failure probe emitted an invalid source path")
        endif()
        set(source_name_marker_length ${source_name_only_suffix_length})
    else()
        set(source_name_marker_length ${source_name_suffix_length})
    endif()
    math(EXPR diagnostic_line_start
        "${source_name_position} + ${source_name_marker_length}")
    string(SUBSTRING "${diagnostic_metadata}" ${diagnostic_line_start} -1
        diagnostic_line_number)
    if(NOT diagnostic_line_number MATCHES "^[1-9][0-9]*$")
        fail_probe("Process failure probe emitted unexpected stdout")
    endif()
    if(diagnostic_line_number IN_LIST diagnostic_line_numbers)
        fail_probe("Process failure probe emitted duplicate line metadata")
    endif()
    list(APPEND diagnostic_line_numbers ${diagnostic_line_number})
    math(EXPR detached_failure_count "${detached_failure_count} + 1")
endwhile()
if(NOT "${detached_failure_count}" EQUAL
   "${PROBE_DETACHED_FAILURE_COUNT}")
    fail_probe("Process failure probe reported an unexpected failure count")
endif()

string(STRIP "${probe_error}" stripped_probe_error)
if(NOT "${stripped_probe_error}" STREQUAL "")
    fail_probe("Process failure probe emitted unexpected stderr")
endif()
