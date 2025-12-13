cmake_minimum_required(VERSION 3.10)
project(qfront LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
file(GLOB_RECURSE SRC_FILES "src/*.cpp" "src/**/*.cpp")
add_executable(qfront
    ${SRC_FILES}
    antlr/parser/qasm3Lexer.cpp
    antlr/parser/qasm3Parser.cpp
)

target_include_directories(qfront PRIVATE
    inc
    antlr/parser
    /usr/local/include
    /usr/local/include/antlr4-runtime
)

target_link_libraries(qfront PRIVATE antlr4-runtime)
