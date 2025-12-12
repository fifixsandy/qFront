cmake_minimum_required(VERSION 3.10)
project(qfront)
set(CMAKE_BUILD_RPATH "/usr/local/lib")
add_executable(qfront
    src/main.cpp
    antlr/parser/qasm3Lexer.cpp
    antlr/parser/qasm3Parser.cpp
)

target_include_directories(qfront PRIVATE /usr/local/include /usr/local/include/antlr4-runtime)

target_link_libraries(qfront PRIVATE antlr4-runtime)
