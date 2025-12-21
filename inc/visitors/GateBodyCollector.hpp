/**
 * @file GateBodyCollector.hpp
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
class GateBodyCollector : public qasm3ParserBaseVisitor {
public:
    GateBodyCollector(IR& ir, ScopeManager& scopes);

    std::any visitGateStatement(qasm3Parser::GateStatementContext *ctx) override;
    std::any visitGateCallStatement(qasm3Parser::GateCallStatementContext* ctx) override;
    std::any visitForStatement(qasm3Parser::ForStatementContext *ctx) override;


private:
    IR& _ir;
    GateDef* current_gate = nullptr;
    std::vector<std::vector<GateStmt>*> body_stack;
    ScopeManager& _scopes;

};

/** EOF GateBodyCollector.hpp */