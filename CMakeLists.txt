cmake_minimum_required(VERSION 3.28)
project(RV32I)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(src/main)

# option(TEST_BUILD "" ON)
# if(BUILD_TEST)
message(STATUS "Building test...")
add_subdirectory(test)
# endif()


