# OpenQASM Frontend (C++)

**Author:** Filip Novák

This is a C++ frontend for parsing and analyzing OpenQASM 3 programs using ANTLR4. It reads OpenQASM 3 source files, parses them using a generated ANTLR4 lexer and parser, builds an internal IR (Intermediate Representation), applies optional transformation passes, and outputs the result in a chosen target format.

### Supported output formats (`--target`)

| Target | Description |
|---|---|
| `openqasm3` | OpenQASM 3.0 |
| `openqasm2` | OpenQASM 2.0 |
| `stim` | Stim circuit format |
| `autoq-para` | AutoQ parametric format |
| `mosf` | MOSF (MoToMEDUSA serialization format) |
| `stats` | Circuit statistics |

### Supported passes

- `--decompose-mcx` — decompose multi-controlled X gates into Toffoli gates
- `--merge-registers` — merge multiple qubit registers into one
- `--eval-angles` — evaluate symbolic rotation angles to numeric values

## Project Structure

- `src/` - Main application source codes.
- `antlr/` - ANTLR grammar files (`qasm3Lexer.g4`, `qasm3Parser.g4`) and generated parser/lexer.
- `circuits/` - Example QASM files.
- `Makefile` - Helper for building the project and ANTLR setup.
- `CMakeLists.txt` - CMake configuration.

## Quick Start

### Build the image

```bash
docker build -t qfront .
```

### Run on a QASM file

```bash
docker run --rm qfront ./qfront circuits/adder-cdkm.qasm
```

### Run interactively

```bash
docker run --rm -it qfront bash
```

### Mount a local circuit file

To run the frontend on a QASM file from your host machine:

```bash
docker run --rm -v /path/to/your/file.qasm:/qfront/circuit.qasm qfront ./qfront circuit.qasm
```

### Development alias

For iterative development, add this alias to your shell config (`~/.bashrc`, `~/.zshrc`, etc.) to mount your current working directory into the container:

```bash
alias qfront-dev='docker run --rm -it -v "$PWD":/qfront qfront:latest /bin/bash'
```

Then simply run:

```bash
qfront-dev
```

This drops you into an interactive shell inside the container with your local files live-mounted at `/qfront`, so any changes you make on the host are immediately reflected inside the container.