# Makefile wrapper for OpenQASM frontend

BUILD_DIR := build
BIN := $(BUILD_DIR)/qfront
ANTLR_JAR := antlr/antlr-4.13.0-complete.jar
PARSER_DIR := antlr/parser
ANTLR_VERSION := 4.13.0

# Default target: build binary
all: $(BIN)

# Build binary with CMake
$(BIN):
	@echo "Building qfront binary with CMake..."
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. 
	cd $(BUILD_DIR) && make
	@mv -f $(BIN) .
	@echo "Moving $(BIN) to root..."

# Clean build directory
clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)
	@echo "Done cleaning build directory."

# Hard clean including ANTLR files
cleanall: clean
	@echo "Removing ANTLR jar, parser, and runtime..."
	rm -rf antlr/antlr-$(ANTLR_VERSION)-complete.jar
	rm -rf $(PARSER_DIR)
	rm -rf antlr/antlr4
	@echo "Full cleanup done."

# Download ANTLR jar and generate Cpp parser/lexer
antlr:
	@echo "Generating lexer..."
	antlr4 \
		-Dlanguage=Cpp \
		-o antlr/parser \
		antlr/qasm3Lexer.g4

	@echo "Generating parser..."
	antlr4 \
		-Dlanguage=Cpp \
		-visitor \
		-lib antlr/parser/antlr\
		-o antlr/parser \
		antlr/qasm3Parser.g4

	@echo "Moving generated files to $(PARSER_DIR)..."
	@mv -f $(PARSER_DIR)/antlr/* $(PARSER_DIR)/ 2>/dev/null || true
	@rm -rf $(PARSER_DIR)/antlr
	@echo "Parser generation complete."

antlr-jar:
	@echo "Downloading ANTLR jar if needed..."
	@test -f $(ANTLR_JAR) || \
			wget -O $(ANTLR_JAR) https://www.antlr.org/download/antlr-$(ANTLR_VERSION)-complete.jar
	@echo "Generating lexer..."
		java -jar $(ANTLR_JAR) \
			-Dlanguage=Cpp \
			-o antlr/parser \
			antlr/qasm3Lexer.g4
	@echo "Generating parser..."
	java -jar $(ANTLR_JAR) \
		-Dlanguage=Cpp \
		-visitor \
		-lib antlr/parser/antlr\
		-o antlr/parser \
		antlr/qasm3Parser.g4
	@echo "Moving generated files to $(PARSER_DIR)..."
	@mv -f $(PARSER_DIR)/antlr/* $(PARSER_DIR)/ 2>/dev/null || true
	@rm -rf $(PARSER_DIR)/antlr

# Build and install ANTLR Cpp runtime
runtime:
	@echo "Cloning and building ANTLR Cpp runtime locally..."
	@mkdir -p antlr
	cd antlr && \
	git clone https://github.com/antlr/antlr4.git || echo "antlr4 already cloned"
	cd antlr/antlr4/runtime/Cpp && \
	mkdir -p build && cd build && \
	cmake .. \
	  -DCMAKE_BUILD_TYPE=Release \
	  -DCMAKE_INSTALL_PREFIX=$$(pwd)/../../../../antlr/install && \
	make && \
	make install
	@echo "ANTLR Cpp runtime installed to antlr/install"

.PHONY: all clean antlr runtime cleanall
