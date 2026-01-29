/**
 * @file GateBodyCollector.cpp
 * @author Filip Novak
 * @date 2025-12-20
 * @brief Visitor methods for collecting gate body statements
 */

#include "../../inc/visitors/ProgramCollector.hpp"
#include "utils.hpp"

std::any ProgramCollector::gateBody_visitGateCallStatement(
    qasm3Parser::GateCallStatementContext* ctx) {
    GatePlacement placement;

    placement.gate_name = ctx->Identifier()->getText();
    auto sym = _scopes.lookupSymbol(placement.gate_name);
    if (!sym || sym->kind != SymbolKind::Gate) {
        throw std::runtime_error(
            "Unknown gate '" + placement.gate_name + "' in gate body");
    }

    if (auto p = std::get_if<size_t>(&sym->ir_ref)) {
        placement.gate_id = *p;
    } else {
        throw std::runtime_error("Internal error: Symbol does not contain a gate ID");
    }

    // mark used for later print of only used gates
    _ir.markGateUsed(placement.gate_id);


    auto operandCtxs = ctx->gateOperandList()->gateOperand();
    for (auto operandCtx : operandCtxs ) {
        // in gate bodies during definition, should always be one of the parameters
        auto* indexed = operandCtx->indexedIdentifier();
        if (!indexed) {
            throw std::runtime_error("Hardware qubits not supported yet");
        }

        std::string operandName = indexed->Identifier()->getText();
        auto sym = _scopes.lookupSymbol(operandName);
        if (!sym || sym->kind != SymbolKind::Qubit) {
            throw std::runtime_error("Unknown qubit argument: " + operandName);
        }

        placement.relativeInputs.push_back(current_gate->argument_index.at(operandName));

    }

    if (body_stack.empty())
        throw std::runtime_error("No active gate body to append gate call");
    body_stack.back()->push_back(std::move(placement));

    return nullptr;
}

std::any ProgramCollector::gateBody_visitForStatement(
    qasm3Parser::ForStatementContext *ctx) {

    RepeatBlock loop;

    if (auto *range = ctx->rangeExpression()) {
        auto exprs = range->expression();
        int start = std::stoi(exprs[0]->getText());
        int end   = std::stoi(exprs.back()->getText());
        int step  = 1;
        if (exprs.size() == 3)
            step = std::stoi(exprs[1]->getText());

        loop.count = (step > 0) ? ((end - start + step) / step)
                                : ((start - end - step) / (-step));
    }
    else if (auto *set = ctx->setExpression()) {
        std::vector<std::string> values;
        for (auto *expr : set->expression())
            values.push_back(expr->getText());
        loop.count = values.size();
    }
    else {
        throw std::runtime_error("Expressions in for loops not supported yet");
    }

    if (body_stack.empty())
        throw std::runtime_error("No active gate body to append repeat block");

    // loop body
    body_stack.push_back(&loop.body);
    visit(ctx->statementOrScope());
    body_stack.pop_back();

    body_stack.back()->push_back(std::move(loop));

    return nullptr;
}

/** EOF GateBodyCollector.cpp */