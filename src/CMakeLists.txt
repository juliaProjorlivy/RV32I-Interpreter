project(${CMAKE_PROJECT_NAME})

add_library(rv32i STATIC decode.cpp execute.cpp translate.cpp io.cpp)

target_link_libraries(rv32i
    PUBLIC
    asmjit::asmjit
    elfio::elfio)

target_include_directories(rv32i
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include)


