#include <iostream>
#include <fstream>
#include <antlr4-runtime/antlr4-runtime.h>
#include "../antlr/parser/qasm3Lexer.h"
#include "../antlr/parser/qasm3Parser.h"
#include "../antlr/parser/qasm3ParserBaseVisitor.h"
#include "../inc/ir.hpp"
#include "../inc/visitors/RegisterCollector.hpp"

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
    parser.removeErrorListeners();

    tree::ParseTree* tree = parser.program();
    //std::cout << tree->toStringTree() << "\n";
    //std::cout << "Parsed successfully!\n";

    IR ir;
    RegisterCollector regColector(ir);
    regColector.visit(tree);

    for (auto r : ir.getAllRegisters()) {
        std::cout << "Name: " << r.name << " Size: " << r.size << " Kind: ";
        if (r.kind == RegisterKind::Parametric) {
            std::cout << "Parametric" << std::endl;
        } else {
            std::cout << "Nonparametric" << std::endl;
        }
    }

    return 0;
}
