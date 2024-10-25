project(${CMAKE_PROJECT_NAME})

add_executable(main main.cpp)

target_compile_options(main PRIVATE ${CMAKE_CXX_FLAGS})

target_link_libraries(main
    PRIVATE
    rv32i
)

