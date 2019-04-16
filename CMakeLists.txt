
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
    ${DIR_INCLUDE}/atomic-queues/mpmc.h

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

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-Wshadow"        HAS_SHADOW_WARNING)
CHECK_CXX_COMPILER_FLAG("-fno-exceptions" HAS_DISABLE_EXCEPTIONS)
CHECK_CXX_COMPILER_FLAG("-fno-rtti"       HAS_DISABLE_RTTI)

if(HAS_SHADOW_WARNING)
    target_compile_options(${PROJECT_TEST} PRIVATE "-Wshadow")
endif()

if(HAS_DISABLE_EXCEPTIONS)
    target_compile_options(${PROJECT_TEST} PRIVATE "-fno-exceptions")
endif()

if(HAS_DISABLE_RTTI)
    target_compile_options(${PROJECT_TEST} PRIVATE "-fno-rtti")
endif()


# RAM: TODO: Get building with sanatisers to work.
#target_compile_options(${PROJECT_TEST} PRIVATE "-fsanitize=address")
#target_compile_options(${PROJECT_TEST} PRIVATE "-fsanitize=thread")
#target_compile_options(${PROJECT_TEST} PRIVATE "-fsanitize=memory")
#target_compile_options(${PROJECT_TEST} PRIVATE "-fsanitize=undefined")

#target_link_libraries(
#    ${PROJECT_TEST}
#    PRIVATE
#        tsan
#)

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
