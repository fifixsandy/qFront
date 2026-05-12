cmake_minimum_required(VERSION 3.14)
project(qfront LANGUAGES CXX)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)

FetchContent_Declare(
    exprtk_src
    GIT_REPOSITORY https://github.com/ArashPartow/exprtk.git
    GIT_TAG master
)

FetchContent_MakeAvailable(json exprtk_src)

add_library(exprtk INTERFACE)
target_include_directories(exprtk INTERFACE
    ${exprtk_src_SOURCE_DIR}
)

find_library(GMP_LIBRARY gmp)

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
    /algebraic-complex-numbers/src
)

target_link_libraries(qfront PRIVATE
    antlr4-runtime
    nlohmann_json::nlohmann_json
    gmp
    ${CMAKE_SOURCE_DIR}/libalgebraic_complex_numbers.a
    exprtk
)