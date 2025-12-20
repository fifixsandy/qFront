/**
 * @file RegisterCollector.cpp
 * @author Filip Novak
 * @date 2025-12-20
 * 
 * Implements the RegisterCollector visitor, which traverses the OpenQASM 3
 * parse tree and populates the IR with register declarations.
 *
 * This visitor is responsible only for collecting register metadata
 * (name, kind, type, and size). Semantic lowering and output-specific
 * transformations are handled in later stages.
 */

#include "../../inc/visitors/RegisterCollector.hpp"
#include "../../inc/utils.hpp"

RegisterCollector::RegisterCollector(IR& ir, ScopeManager& scopes) : _ir(ir), _scopes(scopes) {}

std::any RegisterCollector::visitQuantumDeclarationStatement(
    qasm3Parser::QuantumDeclarationStatementContext *ctx) {
    RegisterDef reg;
    reg.name = ctx->Identifier()->getText();
    reg.type = RegisterType::Qubit;

    auto qt = ctx->qubitType();
    auto designator = qt->designator();

    // there is designator about array size
    if (designator) {
        auto expr = designator->expression();
        if (auto size = parse_utils::tryExtractIntConst(expr)) {
            reg.kind = RegisterKind::Nonparametric;
            reg.size = expr->getText();
        } else {
            reg.kind = RegisterKind::Parametric;
            reg.size = expr->getText();
        }
    } else {
        reg.kind = RegisterKind::Nonparametric;
        reg.size = "1";
    }

    auto id = _ir.addRegister(reg);

    _scopes.addSymbol(Symbol{
        .name = reg.name,
        .kind = SymbolKind::Register,
        .ir_ref = id,
    });


    return nullptr;
}


std::any RegisterCollector::visitClassicalDeclarationStatement(qasm3Parser::ClassicalDeclarationStatementContext *ctx) {
    return nullptr;
}
