if(EXISTS "/home/julia/projects/RV32I-Interpreter/build/test/test[1]_tests.cmake")
  include("/home/julia/projects/RV32I-Interpreter/build/test/test[1]_tests.cmake")
else()
  add_test(test_NOT_BUILT test_NOT_BUILT)
endif()