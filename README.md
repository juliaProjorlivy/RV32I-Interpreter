# RV32I-INTERPRETER   
## Run and install   
[Google test](https://google.github.io/googletest/) is used for testing and [elfio](https://github.com/serge1/ELFIO) for parsing elf files.   
So google test automatically loads when cmake.   
To have elfio you can use `git clone --recurse-submodules`   
There some options to run a program with.    
`-DRUN_DEBUG=ON `- to run with debug options   
`-DRUN_TEST=ON` - to compile tests   
To build:   
```
cmake -S . -B build [<options>]
cmake --build build
```
It takes one argument - rv32i elf file . There is an example in `rv32i/code/examples/fib/`   
T run:   
```
./build/src/main/main some_file
```
To run tests:   
```
cd build/test
ctest
```
or   
```
cd build/test
./test
```
