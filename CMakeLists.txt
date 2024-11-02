cmake_minimum_required(VERSION 3.28)
project(RV32I)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(RUN_DEBUG "" OFF)
if(RUN_DEBUG)
message(STATUS "Compile with debug flags...")
set(CMAKE_CXX_FLAGS "-D _DEBUG -ggdb3 -O0 -Wall -Wextra -Wunused -Wpedantic -Wshadow -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr")
else()
set(CMAKE_CXX_FLAGS "-Wall")
endif()

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(src/main)

option(RUN_TEST "" OFF)
if(RUN_TEST)
message(STATUS "Building test...")
add_subdirectory(test)
endif()


