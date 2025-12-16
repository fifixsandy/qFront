/**
 * @file ProgramCollector.cpp
 * @author Filip Novak
 * @date 2025-12-15
 */

#include "../../inc/visitors/ProgramCollector.hpp"
#include "../../inc/utils.hpp"

ProgramCollector::ProgramCollector(IR& ir) : _ir(ir) {
    program_stack.push_back(&ir.getProgram());
}

std::any ProgramCollector::visitGateStatement(
    qasm3Parser::GateStatementContext *ctx) {
        return nullptr; // skip Gate definition statements
}


std::any ProgramCollector::visitGateCallStatement(
    qasm3Parser::GateCallStatementContext* ctx) {
    GateApplication application;

    auto gate_name = ctx->Identifier()->getText();
    application.gate_id = _ir.getGateId(gate_name);
    auto operandCtxs = ctx->gateOperandList()->gateOperand();

    for (auto* operandCtx : operandCtxs) {
        auto* indexed = operandCtx->indexedIdentifier();
        if (!indexed) {
            throw std::runtime_error("Hardware qubits not supported yet");
        }

        std::string operandName = indexed->Identifier()->getText();

        RegisterRef ref;
        ref.reg_id = _ir.getRegisterId(operandName);

        if (!indexed->indexOperator().empty()) {
            auto firstIndexOp = indexed->indexOperator(0); // pick first IndexOperatorContext
            auto expressions = firstIndexOp->expression();  // this is a vector<ExpressionContext*>

            if (!expressions.empty()) {
                ref.qubit_index = expressions[0]->getText();
            } else {
                throw std::runtime_error("Index operator has no expression");
            }
        } else {
            ref.qubit_index = "0";
        }

        application.operands.push_back(ref);

    }

    auto applicationPtr = std::make_unique<GateApplication>(std::move(application));
    program_stack.back()->push_back(std::move(applicationPtr));

    return nullptr;
}


std::any ProgramCollector::visitForStatement(
    qasm3Parser::ForStatementContext *ctx) {

    auto loop = std::make_unique<LoopApplication>();

    loop->type = ctx->scalarType()->getText();
    loop->variable = ctx->Identifier()->getText();
    if (auto *range = ctx->rangeExpression()) {
        Interval interval;

        auto exprs = range->expression();

        interval.start = exprs[0]->getText();
        interval.end   = exprs.back()->getText();

        if (exprs.size() == 3)
            interval.step = exprs[1]->getText();

        loop->values = interval;
        loop->values = interval;
    }
    else if (auto *set = ctx->setExpression()) {
        std::vector<std::string> values;
        for (auto *expr : set->expression())
            values.push_back(expr->getText());

        loop->values = values;
    }
    else {
        loop->values = ctx->expression()->getText();
    }

    program_stack.push_back(&loop->body);
    visit(ctx->statementOrScope());
    program_stack.pop_back();

    program_stack.back()->push_back(std::move(loop));
    return nullptr;
}

