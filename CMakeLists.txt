cmake_minimum_required(VERSION 3.1)  # target_sources
project(amblaq)
enable_testing()

set(DIR_SOURCE  ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(DIR_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(DIR_TESTS   ${CMAKE_CURRENT_SOURCE_DIR}/tests)

set(PROJECT_TEST ${PROJECT_NAME}_test)
# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
option(SANATIZE_THREAD "Use thread sanatizer" OFF)

# ------------------------------------------------------------------------------
# Files
# ------------------------------------------------------------------------------
set(SOURCE
    ${DIR_INCLUDE}/amblaq/queues.h
)

set(SOURCE_TESTS
    ${DIR_TESTS}/test_queues.c
)

set(SOURCE_MISC
    ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    ${CMAKE_CURRENT_SOURCE_DIR}/.travis.yml
)

# ------------------------------------------------------------------------------
# Binaries
# ------------------------------------------------------------------------------
add_library   (${PROJECT_NAME} INTERFACE)
add_executable(${PROJECT_TEST} ${SOURCE_TESTS})

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE
        ${DIR_INCLUDE}
)

target_include_directories(
    ${PROJECT_TEST}
    PRIVATE
        ${DIR_TESTS}
)

target_sources(${PROJECT_NAME} INTERFACE ${SOURCE})
target_sources(${PROJECT_NAME} INTERFACE ${SOURCE_MISC})

add_test(${PROJECT_TEST} ${PROJECT_TEST})

# ------------------------------------------------------------------------------
# Properties
# ------------------------------------------------------------------------------
set_target_properties(
    ${PROJECT_TEST}
    PROPERTIES
        C_STANDARD            11
        C_STANDARD_REQUIRED   ON
        C_EXTENSIONS          OFF

        CXX_STANDARD          11
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS        OFF
)

# ------------------------------------------------------------------------------
# Compiler flags
# ------------------------------------------------------------------------------
include(CheckCCompilerFlag)

function(private_c_flags target flag)
    string(REGEX REPLACE "^-Wno-" "-W" checked_flag ${flag})
    set(flag_name ${checked_flag})

    string(REPLACE       "+"     "X" flag_name ${flag_name})
    string(REGEX REPLACE "[-=/]" "_" flag_name ${flag_name})

    check_c_compiler_flag(${checked_flag} has${flag_name})

    if(has${flag_name})
        target_compile_options(${target} PRIVATE ${checked_flag})
    endif()
endfunction()

# ------------------------------------------------------------------------------

private_c_flags(${PROJECT_TEST} "-Wall")
private_c_flags(${PROJECT_TEST} "/W4")
private_c_flags(${PROJECT_TEST} "-Wshadow")

private_c_flags(${PROJECT_TEST} "/TP")
# Build c files as c++, otherwise they build as C90 :-(

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------
target_link_libraries(
    ${PROJECT_TEST}
    PRIVATE
        ${PROJECT_NAME}
)

if (UNIX)
    find_package(Threads REQUIRED)

    target_link_libraries(
        ${PROJECT_TEST}
        PRIVATE
            ${CMAKE_THREAD_LIBS_INIT}
    )
endif()
