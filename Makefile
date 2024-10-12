CXX = g++

CXX_FLAGS = -I ./include/ -I /usr/include/ -D _DEBUG -ggdb3 -std=c++20 -O0 -Wall -Wextra  -Wunused -Wpedantic -Wshadow -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

#variables for arc-algorithm
SRCS = src/main.cpp src/decode.cpp src/execute.cpp
OBJ = $(patsubst %.cpp, build/%.o, $(subst src/, , $(SRCS))) 
EXECUTABLE = play

all: $(OBJ)
	@echo "CXX $(EXECUTABLE)"
	@$(CXX) $(CXX_FLAGS) $(OBJ) -o $(EXECUTABLE)
build/%.o: src/%.cpp
	mkdir -p ./build
	@$(CXX) $(CXX_FLAGS) -c -o $@ $<
.PHONY: clean mem tclean

clean:
	@rm -f build/*.o
	@rm -f $(EXECUTABLE)
mem:
	valgrind --leak-check=full --leak-resolution=med ./$(EXECUTABLE)
	
