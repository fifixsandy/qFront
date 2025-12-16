#include <iostream>
#include <fstream>
#include <antlr4-runtime/antlr4-runtime.h>
#include "../antlr/parser/qasm3Lexer.h"
#include "../antlr/parser/qasm3Parser.h"
#include "../antlr/parser/qasm3ParserBaseVisitor.h"
#include "../inc/ir.hpp"
#include "../inc/visitors/RegisterCollector.hpp"
#include "../inc/visitors/GateHeadersCollector.hpp"
#include "../inc/visitors/GateBodyCollector.hpp"
#include "../inc/visitors/ProgramCollector.hpp"
#include "../inc/temp_printer.hpp"

using namespace antlr4;

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./qfront <file.qasm>\n";
        return 1;
    }

    std::ifstream stream(argv[1]);
    if (!stream.good()) {
        std::cerr << "Could not open file: " << argv[1] << "\n";
        return 1;
    }

    ANTLRInputStream input(stream);
    qasm3Lexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    qasm3Parser parser(&tokens);

    tree::ParseTree* tree = parser.program();
    //std::cout << tree->toStringTree() << "\n";
    //std::cout << "Parsed successfully!\n";

    IR ir;
    RegisterCollector regColector(ir);
    regColector.visit(tree);
    GateHeadersCollector gateCollector(ir);
    gateCollector.visit(tree);
    GateBodyCollector gateBodyCollector(ir);
    gateBodyCollector.visit(tree);
    ProgramCollector programCollector(ir);
    programCollector.visit(tree);

    std::cout << "ParametricProgram program = {\n";
    printGateTable(ir);
    printRegisterTable(ir);
    printTransducerDefs(ir);
    printProgram(ir);
    std::cout << "}";
    return 0;
}
