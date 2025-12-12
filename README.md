# OpenQASM Frontend (C++)

**Author:** Filip Novák

This is a C++ frontend for parsing and analyzing OpenQASM 3 programs using ANTLR4.

## Project Structure

- `src/` - Main application source codes.
- `antlr/` - ANTLR grammar files (`qasm3Lexer.g4`, `qasm3Parser.g4`) and generated parser/lexer.
- `circuits/` - Example QASM files.
- `Makefile` - Helper for building the project and ANTLR setup.
- `CMakeLists.txt` - CMake configuration.

## Prerequisites

- CMake >= 3.10  
- ANTLR4 Java tool (downloaded automatically via `Makefile`)  
- ANTLR4 C++ runtime (install via `make runtime`)  
- Java (for ANTLR)  
- GCC

## Quick Start

1. Build ANTLR runtime (C++) and install it:
    ```bash
    (sudo) make runtime
    ```

2. Generate parser and lexer:
    ```bash
    make antlr
    ```

3. Build the frontend:
    ```bash
    make
    ```

4. Run on a QASM file:
    ```bash
    ./qfront circuits/adder-cdkm.qasm
    ```