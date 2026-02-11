cmake_minimum_required(VERSION 3.10)
project(qfront LANGUAGES CXX)
set ( CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

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
    /usr/include/antlr4-runtime
)

target_link_libraries(qfront PRIVATE antlr4-runtime nlohmann_json::nlohmann_json)
