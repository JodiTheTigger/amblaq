
cmake_minimum_required(VERSION 3.0.0)
project(queues2 C)

set(DIR_SOURCE src)

# ------------------------------------------------------------------------------
# Files
# ------------------------------------------------------------------------------
set(SOURCE
    ${DIR_SOURCE}/mpmc.c
)

# ------------------------------------------------------------------------------
# Binaries
# ------------------------------------------------------------------------------
add_library(${PROJECT_NAME} ${SOURCE})

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        ${DIR_SOURCE}   
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
