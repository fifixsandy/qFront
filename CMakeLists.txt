cmake_minimum_required(VERSION 3.10)
project(qfront LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)

FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)
FetchContent_MakeAvailable(json)

set(ANTLR_INSTALL_DIR ${CMAKE_SOURCE_DIR}/antlr/antlr/install)

set(ANTLR4_INCLUDE_DIR ${ANTLR_INSTALL_DIR}/include/antlr4-runtime)

find_library(ANTLR4_RUNTIME_LIB
    NAMES antlr4-runtime
    PATHS ${ANTLR_INSTALL_DIR}/lib
    NO_DEFAULT_PATH
)

if(NOT ANTLR4_RUNTIME_LIB)
    message(FATAL_ERROR "antlr4-runtime library not found. Run: make runtime")
endif()

file(GLOB_RECURSE SRC_FILES
    src/*.cpp
    src/**/*.cpp
)

add_executable(qfront
    ${SRC_FILES}
    antlr/parser/qasm3Lexer.cpp
    antlr/parser/qasm3Parser.cpp
)

target_include_directories(qfront PRIVATE
    inc
    antlr/parser
    ${ANTLR4_INCLUDE_DIR}
)

target_link_libraries(qfront PRIVATE
    ${ANTLR4_RUNTIME_LIB}
    nlohmann_json::nlohmann_json
)

set_target_properties(qfront PROPERTIES
    BUILD_RPATH "$ORIGIN/../antlr/install/lib"
    INSTALL_RPATH "$ORIGIN/../antlr/install/lib"
)
