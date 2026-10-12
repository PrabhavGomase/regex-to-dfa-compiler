cmake_minimum_required(VERSION 3.12)
project(RegexLab LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_library(regexlab_core
  src/lexer/lexer.cpp src/parser/parser.cpp src/ast/ast.cpp src/nfa/nfa.cpp
  src/dfa/dfa.cpp src/minimization/minimizer.cpp src/matcher/matcher.cpp
  src/direct/direct.cpp src/analysis/equivalence.cpp src/analysis/optimizer.cpp
  src/analysis/benchmark.cpp src/compiler/compiler.cpp src/compiler/json.cpp)
if(MSVC)
  target_compile_options(regexlab_core PRIVATE /W4)
else()
  target_compile_options(regexlab_core PRIVATE -Wall -Wextra -pedantic)
endif()
add_executable(regexlab src/main.cpp)
target_link_libraries(regexlab PRIVATE regexlab_core)
add_executable(compiler_tests tests/compiler_tests.cpp)
target_link_libraries(compiler_tests PRIVATE regexlab_core)
enable_testing()
add_test(NAME compiler_correctness COMMAND compiler_tests)
