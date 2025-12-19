/**
 * @file RegisterCollector.hpp
 * @author Filip Novak
 * @date 2025-12-13
 */

#pragma once
#include "antlr4-runtime.h"
#include "../antlr/parser/qasm3Lexer.h"
#include "../antlr/parser/qasm3Parser.h"
#include "../antlr/parser/qasm3ParserBaseVisitor.h"
#include "ir.hpp"

class RegisterCollector : public qasm3ParserBaseVisitor {
public:
    RegisterCollector(IR& ir);

    std::any visitQuantumDeclarationStatement(qasm3Parser::QuantumDeclarationStatementContext *ctx) override;
    std::any visitClassicalDeclarationStatement(qasm3Parser::ClassicalDeclarationStatementContext *ctx) override;

private:
    IR& _ir;
};

/** EOF RegisterCollector.hpp */