/**
 * @file RegisterCollector.hpp
 * @author Filip Novak
 * @date 2025-12-20
 */

#pragma once
#include <antlr4-runtime/antlr4-runtime.h>
#include "qasm3Lexer.h"
#include "qasm3Parser.h"
#include "qasm3ParserBaseVisitor.h"
#include "ir.hpp"
#include "ScopeManager.hpp"
class RegisterCollector : public qasm3ParserBaseVisitor {
public:
    RegisterCollector(IR& ir, ScopeManager& scopes);

    std::any visitQuantumDeclarationStatement(qasm3Parser::QuantumDeclarationStatementContext *ctx) override;
    std::any visitClassicalDeclarationStatement(qasm3Parser::ClassicalDeclarationStatementContext *ctx) override;

private:
    IR& _ir;
    ScopeManager& _scopes;

};

/** EOF RegisterCollector.hpp */