# Use of this file:
# cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-llvm.cmake ..

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER    clang)
SET(CMAKE_CXX_COMPILER  clang++)

SET(ENABLE_LLVM CACHE BOOL TRUE)
SET(ENABLE_LLVM_BC CACHE BOOL FALSE)

IF ( ENABLE_LLVM_BC )
  #TODO: make this work...
  #this only crates the llvm objects, it can't link them together currently
  SET(CMAKE_C_FLAGS "-emit-llvm")
  SET(CMAKE_CXX_FLAGS "-emit-llvm")
ENDIF ( ENABLE_LLVM_BC )

