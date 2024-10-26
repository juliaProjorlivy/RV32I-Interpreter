project(${CMAKE_PROJECT_NAME})

add_executable(main main.cpp)

target_compile_options(main PRIVATE ${CMAKE_CXX_FLAGS})

find_path(ELFIO_PATH elfio.hpp
PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../ELFIO/elfio)

if(ELFIO_PATH)
target_include_directories(rv32i
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ELFIO)
endif()

target_link_libraries(main
    PRIVATE
    rv32i
)

