/**
 * @file GateBodyCollector.cpp
 * @author Filip Novak
 * @date 2025-12-13
 * 
 * Implements the GateBodyCollector visitor, which traverses the OpenQASM 3
 * parse tree and populates the IR gates with the bodies.
 */

#include "../../inc/visitors/GateBodyCollector.hpp"
#include "../../inc/utils.hpp"

GateBodyCollector::GateBodyCollector(IR& ir) : _ir(ir) {}

std::any GateBodyCollector::visitGateStatement(
    qasm3Parser::GateStatementContext *ctx) {
    current_gate = &_ir.getGate(ctx->Identifier()->getText());

    visit(ctx->scope());
    current_gate = nullptr;
    return nullptr;
}


std::any GateBodyCollector::visitGateCallStatement(
    qasm3Parser::GateCallStatementContext* ctx) {
    if (!current_gate) {return nullptr;} // gate call not in gate body definition
    GatePlacement placement;

    placement.gate_name = ctx->Identifier()->getText();
    placement.gate_id = _ir.getGateId(placement.gate_name);
    _ir.markGateUsed(placement.gate_id);
    auto operandCtxs = ctx->gateOperandList()->gateOperand();

    for (auto operandCtx : operandCtxs ) {
        // in gate bodies during definition, should always be one of the parameters
        auto* indexed = operandCtx->indexedIdentifier();
        if (!indexed) {
            throw std::runtime_error("Hardware qubits not supported yet");
        }

        std::string operandName = indexed->Identifier()->getText();
        auto it = current_gate->argument_index.find(operandName);
        if (it != current_gate->argument_index.end()) {
            placement.relativeInputs.push_back(it->second);
        } else {
            throw std::runtime_error("Unknown qubit argument:*" + operandName + "*");
        }
    }

    auto& body = std::get<CompositeGateBody>(current_gate->semantics).body;
    body.push_back(std::move(placement));

    return nullptr;
}
