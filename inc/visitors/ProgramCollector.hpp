/**
 * @file ProgramCollector.hpp
 * @author Filip Novak
 * @date 2025-12-27
 */

#pragma once
#include <antlr4-runtime/antlr4-runtime.h>
#include "qasm3Lexer.h"
#include "qasm3Parser.h"
#include "qasm3ParserBaseVisitor.h"
#include "ir.hpp"
#include "ScopeManager.hpp"
class ProgramCollector : public qasm3ParserBaseVisitor {
public:
    ProgramCollector(IR& ir, ScopeManager& scopes);

    std::any visitGateCallStatement(qasm3Parser::GateCallStatementContext *ctx) override;
    std::any visitGateStatement(qasm3Parser::GateStatementContext *ctx) override;
    std::any visitForStatement(qasm3Parser::ForStatementContext *ctx) override;

    std::any visitDefStatement(qasm3Parser::DefStatementContext* ctx) override;

    std::any inProgram_visitGateCallStatement(qasm3Parser::GateCallStatementContext* ctx);
    std::any gateBody_visitGateCallStatement(qasm3Parser::GateCallStatementContext* ctx);
    std::any inProgram_visitForStatement(qasm3Parser::ForStatementContext *ctx);
    std::any gateBody_visitForStatement(qasm3Parser::ForStatementContext *ctx);
    std::any visitConstDeclarationStatement(qasm3Parser::ConstDeclarationStatementContext *ctx) override;
    std::any visitQuantumDeclarationStatement(qasm3Parser::QuantumDeclarationStatementContext *ctx) override;
    std::any visitOldStyleDeclarationStatement(qasm3Parser::OldStyleDeclarationStatementContext *ctx) override;
private:
    IR& _ir;
    ScopeManager& _scopes;
    GateDef* current_gate = nullptr;
    std::vector<Block*> block_stack;
    std::vector<std::vector<GateStmt>*> body_stack;
};

/** EOF ProgramCollector.hpp */