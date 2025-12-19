/**
 * @file GateBodyCollector.hpp
 * @author Filip Novak
 * @date 2025-12-13
 */

#pragma once
#include "antlr4-runtime.h"
#include "../antlr/parser/qasm3Lexer.h"
#include "../antlr/parser/qasm3Parser.h"
#include "../antlr/parser/qasm3ParserBaseVisitor.h"
#include "ir.hpp"

class GateBodyCollector : public qasm3ParserBaseVisitor {
public:
    GateBodyCollector(IR& ir);

    std::any visitGateStatement(qasm3Parser::GateStatementContext *ctx) override;
    std::any visitGateCallStatement(qasm3Parser::GateCallStatementContext* ctx) override ;

private:
    IR& _ir;
    GateDef* current_gate = nullptr; // TODO: chaange to scope
};

/** EOF GateBodyCollector.hpp */