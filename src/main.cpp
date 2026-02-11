// main.cpp
#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
#include <antlr4-runtime/antlr4-runtime.h>
#include "../antlr/parser/qasm3Lexer.h"
#include "../antlr/parser/qasm3Parser.h"
#include "../antlr/parser/qasm3ParserBaseVisitor.h"
#include "../inc/ir.hpp"
#include "../inc/visitors/GateHeadersCollector.hpp"
#include "../inc/visitors/ProgramCollector.hpp"
#include "../inc/AtomicGateLoader.hpp"
#include "../inc/ScopeManager.hpp"
#include "../inc/printers/Printer.hpp"
#include "../inc/printers/StimPrinter.hpp"
#include "../inc/printers/AutoQParaPrinter.hpp"
#include "../inc/ArgParser.hpp"


using namespace antlr4;


std::unique_ptr<Printer> selectPrinter(const std::string& target) {
    if (target == "stim") {
        return std::make_unique<StimPrinter>();
    } 
    else if (target == "autoq-para") {
        return std::make_unique<AutoQParaPrinter>();
    }

    throw std::runtime_error("Unknown target: " + target);
}


int main(int argc, const char* argv[]) {
    ArgParser::Args args;
    try {
        args = ArgParser::parse(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        ArgParser::printUsage(argv[0]);
        return 1;
    }

    std::istream* input_ptr = &std::cin;
    std::ifstream input_file_stream;

    if (!args.input_file.empty()) {
        input_file_stream.open(args.input_file);
        if (!input_file_stream.good()) {
            std::cerr << "Error: Could not open file: " << args.input_file << "\n";
            return 1;
        }
        input_ptr = &input_file_stream;
    }

    ANTLRInputStream input(*input_ptr);
    qasm3Lexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    qasm3Parser parser(&tokens);

    tree::ParseTree* tree = nullptr;
    try {
        tree = parser.program();
        if (!tree) {
            std::cerr << "Error: Failed to parse program\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during ANTLR parsing: " << e.what() << "\n";
        return 1;
    }

    IR ir;
    ScopeManager scopes;

    try {
        auto gates = loadGates("gates.json");
        for (const auto& gate : gates) {
            auto id = ir.addGate(gate);
            Symbol sym;
            sym.name = gate.name;
            sym.aliases = gate.aliases;
            sym.ir_ref = id;
            sym.kind = SymbolKind::Gate;
            scopes.addSymbol(std::move(sym));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading gates: " << e.what() << "\n";
        return 1;
    }

    // Visitor passes
    try {
        GateHeadersCollector gate_collector(ir, scopes);
        gate_collector.visit(tree);

        ProgramCollector program_collector(ir, scopes);
        program_collector.visit(tree);
    } catch (const std::exception& e) {
        std::cerr << "Error during IR construction: " << e.what() << "\n";
        return 1;
    }

    std::unique_ptr<Printer> printer;
    try {
        printer = selectPrinter(args.target);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    std::ostream* output_ptr = &std::cout;
    std::ofstream output_file_stream;

    if (!args.output_file.empty()) {
        output_file_stream.open(args.output_file);
        if (!output_file_stream.good()) {
            std::cerr << "Error: Could not open output file: " << args.output_file << "\n";
            return 1;
        }
        output_ptr = &output_file_stream;
    }

    try {
        printer->print(ir, *output_ptr);
    } catch (const std::exception& e) {
        std::cerr << "Error during output generation: " << e.what() << "\n";
        return 1;
    }

    if (!args.output_file.empty()) {
        output_file_stream.close();
    }

    return 0;
}
