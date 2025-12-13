/**
 * @file GateHeadersCollector.cpp
 * @author Filip Novak
 * @date 2025-12-13
 * 
 * Implements the GateHeadersCollector visitor, which traverses the OpenQASM 3
 * parse tree and populates the IR with gate headers.
 */

#include "../../inc/visitors/GateHeadersCollector.hpp"
#include "../../inc/utils.hpp"

GateHeadersCollector::GateHeadersCollector(IR& ir) : _ir(ir) {}

std::any GateHeadersCollector::visitGateStatement(
    qasm3Parser::GateStatementContext *ctx) {
    
    GateDef gate;
    gate.name = ctx->Identifier()->getText();
    gate.kind = GateKind::Composite;

    auto qubitsCtx = ctx->qubits;
    if (!qubitsCtx) {
        throw std::runtime_error("Gate has no qubit arguments");
    }

    for (auto* idNode : qubitsCtx->Identifier()) {
        std::string name = idNode->getText();

        std::size_t index = gate.argument_qubits.size();
        gate.argument_qubits.push_back(name);
        gate.argument_index[name] = index;
    }

    if (ctx->params) {
        for (auto* idNode : ctx->params->Identifier()) {
            std::string name = idNode->getText();

            std::size_t index = gate.parameters.size();
            gate.parameters.push_back(name);
            gate.parameter_index[name] = index;
        }
    }

    gate.semantics = CompositeGateBody{};
    _ir.addGate(gate);
    return nullptr;
}

/* EOF GateHeadersCollector*/
