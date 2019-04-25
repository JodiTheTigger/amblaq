
cmake_minimum_required(VERSION 3.1)  # target_sources
project(queues2)

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
    ${DIR_INCLUDE}/queues/queues.h
)

set(SOURCE_TESTS
    ${DIR_TESTS}/test_mpmc.cpp
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
include(CheckCXXCompilerFlag)

function(private_cxx_flags target flag)
    check_cxx_compiler_flag(flag HAS_FLAG)
    if(HAS_FLAG)
        target_compile_options(target PRIVATE flag)
    endif()
endfunction()

# ------------------------------------------------------------------------------

private_cxx_flags(${PROJECT_TEST} "-Wshadow")
private_cxx_flags(${PROJECT_TEST} "-fno-exceptions")
private_cxx_flags(${PROJECT_TEST} "-fno-rtti")

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
