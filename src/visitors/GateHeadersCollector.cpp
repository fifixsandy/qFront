/**
 * @file GateHeadersCollector.cpp
 * @author Filip Novak
 * @date 2025-12-20
 * 
 * Implements the GateHeadersCollector visitor, which traverses the OpenQASM 3
 * parse tree and populates the IR with gate headers.
 */

#include "../../inc/visitors/GateHeadersCollector.hpp"
#include "../../inc/utils.hpp"

GateHeadersCollector::GateHeadersCollector(IR& ir, ScopeManager& scopes) : _ir(ir), _scopes(scopes) {}

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

    auto id = _ir.addGate(gate);

    _scopes.addSymbol(Symbol{
        .name = gate.name,
        .kind = SymbolKind::Gate,
        .ir_ref = id,
    });
    
    return nullptr;
}

std::any GateHeadersCollector::visitDefStatement(
    qasm3Parser::DefStatementContext* ctx) {
    
    SubroutineDef subroutine;
    subroutine.name = ctx->Identifier()->getText();
    if (ctx->argumentDefinitionList()) {
        for (auto* paramCtx : ctx->argumentDefinitionList()->argumentDefinition()) {
            ParameterDef param;
            param.name = paramCtx->Identifier()->getText();

            if (paramCtx->scalarType()) {
                param.type = paramCtx->scalarType()->getText();
            }
            else if (paramCtx->qubitType() || paramCtx->QREG()) {
                param.type = "qubit";
            }
            else if (paramCtx->arrayReferenceType()) {
                param.type = paramCtx->arrayReferenceType()->getText();
            }
            else {
                throw std::runtime_error("Unsupported argument type in subroutine");
            }

            subroutine.parameters.push_back(param);
        }
    }
    if (ctx->returnSignature()) {
        subroutine.return_type = ctx->returnSignature()->scalarType()->getText();
    }

    std::string name = ctx->Identifier()->getText();
    auto id = _ir.addSubroutine(std::move(subroutine));

    _scopes.addSymbol(Symbol{
        .name = name,
        .kind = SymbolKind::Subroutine,
        .ir_ref = id,
    });

    return nullptr;
}

/* EOF GateHeadersCollector*/
