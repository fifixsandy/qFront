/**
 * @file ProgramCollector.hpp
 * @author Filip Novak
 * @date 2025-12-14
 */

#pragma once
#include "antlr4-runtime.h"
#include "../antlr/parser/qasm3Lexer.h"
#include "../antlr/parser/qasm3Parser.h"
#include "../antlr/parser/qasm3ParserBaseVisitor.h"
#include "ir.hpp"

class ProgramCollector : public qasm3ParserBaseVisitor {
public:
    ProgramCollector(IR& ir);

    std::any visitGateCallStatement(qasm3Parser::GateCallStatementContext *ctx) override;
    std::any visitGateStatement(qasm3Parser::GateStatementContext *ctx) override;
    std::any visitForStatement(qasm3Parser::ForStatementContext *ctx) override;

private:
    IR& _ir;
    std::vector<std::vector<ProgramNodePtr>*> program_stack;
};

/** EOF ProgramCollector.hpp */