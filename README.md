# RV32I-INTERPRETER   
## Run and install   
[Google test](https://google.github.io/googletest/) is used for testing and [elfio](https://github.com/serge1/ELFIO) for parsing elf files.   
To build:   
```
conan install conanfile.txt --build=missing
cmake -S . -B build/Release --toolchain build/Release/generators/conan_toolchain.cmake  -DCMAKE_BUILD_TYPE=Release
cmake --build build/Release
```
It takes one argument - rv32i elf file . There is an example in `rv32i/code/examples/fib/`   
T run:   
```
./build/Release/src/main/main some_file
```
To run tests:   
```
cd build/Release/test
ctest
```
or   
```
./build/Release/test/test
```
