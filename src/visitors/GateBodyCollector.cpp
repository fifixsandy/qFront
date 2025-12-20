/**
 * @file GateBodyCollector.cpp
 * @author Filip Novak
 * @date 2025-12-20
 * 
 * Implements the GateBodyCollector visitor, which traverses the OpenQASM 3
 * parse tree and populates the IR gates with the bodies.
 */

#include "../../inc/visitors/GateBodyCollector.hpp"
#include "utils.hpp"

GateBodyCollector::GateBodyCollector(IR& ir, ScopeManager& scopes) : _ir(ir), _scopes(scopes) {}

std::any GateBodyCollector::visitGateStatement(
    qasm3Parser::GateStatementContext *ctx) {
    current_gate = &_ir.getGate(ctx->Identifier()->getText());
    _scopes.enterScope(ScopeKind::GateOrSubroutine);

    // adding qubit identifiers to current scope - they can shadow
    // symbols from outer scopes
    for (const auto& q : current_gate->argument_qubits) {
        _scopes.addSymbol(Symbol{
            .name = q,
            .kind = SymbolKind::Qubit,
        });
    }

    // adding parameter identifiers to current scope - they can shadow
    // symbols from outer scopes
    for (const auto& p : current_gate->parameters) {
        _scopes.addSymbol(Symbol{
            .name = p,
            .kind = SymbolKind::Parameter,
        });
    }

    // handle body
    visit(ctx->scope());

    // leaving, reset scopes
    current_gate = nullptr;
    _scopes.exitScope();
    return nullptr;
}


std::any GateBodyCollector::visitGateCallStatement(
    qasm3Parser::GateCallStatementContext* ctx) {
    if (!current_gate) {return nullptr;} // gate call not in gate body definition
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

    auto& body = std::get<CompositeGateBody>(current_gate->semantics).body;
    body.push_back(std::move(placement));

    return nullptr;
}
