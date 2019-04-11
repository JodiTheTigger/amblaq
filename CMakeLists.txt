
cmake_minimum_required(VERSION 3.0.0)
project(queues2)

set(DIR_SOURCE  src)
set(DIR_INCLUDE include)
set(DIR_TESTS   tests)

set(PROJECT_TEST ${PROJECT_NAME}_test)

# ------------------------------------------------------------------------------
# Files
# ------------------------------------------------------------------------------
set(SOURCE
    ${DIR_INCLUDE}/mpmc.h

    ${DIR_SOURCE}/mpmc.c
)

set(SOURCE_TESTS
    ${DIR_TESTS}/test_mpmc.cpp
)

# ------------------------------------------------------------------------------
# Binaries
# ------------------------------------------------------------------------------
add_library   (${PROJECT_NAME} ${SOURCE})
add_executable(${PROJECT_TEST} ${SOURCE_TESTS})

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        ${DIR_INCLUDE}
    PRIVATE
        ${DIR_SOURCE}
)

target_include_directories(
    ${PROJECT_TEST}
    PRIVATE
        ${DIR_TESTS}
)

# ------------------------------------------------------------------------------
# Properties
# ------------------------------------------------------------------------------
set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        C_STANDARD          11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS        OFF
)

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
# Dependencies
# ------------------------------------------------------------------------------
target_link_libraries(
    ${PROJECT_TEST}
    PRIVATE
        ${PROJECT_NAME}
)
