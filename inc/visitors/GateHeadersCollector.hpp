/**
 * @file GateHeadersCollector.hpp
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

class GateHeadersCollector : public qasm3ParserBaseVisitor {
public:
    GateHeadersCollector(IR& ir, ScopeManager& scopes);

    std::any visitGateStatement(qasm3Parser::GateStatementContext *ctx) override;
    std::any visitDefStatement(qasm3Parser::DefStatementContext* ctx) override;

private:
    IR& _ir;
    ScopeManager& _scopes;
};

/** EOF GateHeadersCollector.hpp */