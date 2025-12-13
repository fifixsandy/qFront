/**
 * @file GateHeadersCollector.hpp
 * @author Filip Novak
 * @date 2025-12-13
 */

#pragma once
#include <antlr4-runtime/antlr4-runtime.h>
#include "../antlr/parser/qasm3Lexer.h"
#include "../antlr/parser/qasm3Parser.h"
#include "../antlr/parser/qasm3ParserBaseVisitor.h"
#include "ir.hpp"

class GateHeadersCollector : public qasm3ParserBaseVisitor {
public:
    GateHeadersCollector(IR& ir);

    std::any visitGateStatement(qasm3Parser::GateStatementContext *ctx) override;

private:
    IR& _ir;
};

/** EOF GateHeadersCollector.hpp */