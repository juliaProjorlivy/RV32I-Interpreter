cmake_minimum_required(VERSION 3.28)
project(RV32I)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(TRACING "" OFF)
if(TRACING)
    add_compile_definitions(TRACING)
    message(STATUS "Building with trace...")
endif()

find_package(elfio REQUIRED)
find_package(asmjit REQUIRED)
# target_include_directories(elfio::elfio PUBLIC ${ELFIO_INCLUDE_DIRS})
# message(STATUS ${ELFIO_INCLUDE_DIRS})
# target_include_directories(asmjit::asmjit PUBLIC ${ASMJIT_INCLUDE_DIRS})

add_subdirectory(src)
add_subdirectory(src/main)

option(RUN_TEST "" ON)
if(RUN_TEST)
find_package(GTest REQUIRED)
# target_include_directories(GTest::GTest PUBLIC ${GTEST_INCLUDE_DIR})
message(STATUS "Building test...")
add_subdirectory(test)
endif()


