project(${CMAKE_PROJECT_NAME})

# include(FetchContent)
# FetchContent_Declare(
#   googletest
#   URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
# )
#
# FetchContent_GetProperties(googletest)
# if(NOT googletest_POPULATED)
#     FetchContent_Populate(googletest)
#     add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
# endif()
#
# FetchContent_MakeAvailable(googletest)
# add_library(GTest::GTest INTERFACE IMPORTED)

# Define tests
enable_testing()

add_executable(test test_execute.cpp test_decode.cpp test_translate.cpp main.cpp)

target_link_libraries(test
    PRIVATE
    rv32i
    pthread
    gtest
    gmock
    gtest_main
)

target_include_directories(test
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/)

include(GoogleTest)
gtest_discover_tests(test)
