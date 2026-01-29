/**
 * @file RegisterCollector.cpp
 * @author Filip Novak
 * @date 2025-12-20
 * @brief Visitor methods for collecting register declarations
 */

#include "../../inc/visitors/ProgramCollector.hpp"
#include "../../inc/utils.hpp"

std::any ProgramCollector::visitQuantumDeclarationStatement(
    qasm3Parser::QuantumDeclarationStatementContext *ctx) {

    if (current_gate != nullptr) {
        throw std::runtime_error(
            "Quantum register declarations are not allowed inside gate bodies");
    }

    RegisterDef reg;
    reg.name = ctx->Identifier()->getText();
    reg.type = RegisterType::Qubit;

    auto qt = ctx->qubitType();
    auto designator = qt->designator();

    // there is designator about array size
    if (designator) {
        auto expr = designator->expression();
        // literal size
        if (auto size = parse_utils::tryExtractIntConst(expr)) {
            reg.kind = RegisterKind::Nonparametric;
            reg.size = expr->getText();
        } else {
            // find symbol to check if it's a const variable
            auto* sym = _scopes.lookupSymbol(expr->getText());

            if (!sym || sym->kind != SymbolKind::ConstVar) {
                throw std::runtime_error(
                    "Cannot determine constant value of register size from \""
                     + expr->getText() + "\".");
            }

            // get variable from IR to check if it has compile time value
            auto variable = _ir.getGlobalVariable(expr->getText());
            if (variable.compile_time_value.empty()) {
                reg.kind = RegisterKind::Parametric;
                reg.size = expr->getText();
            } else {
                reg.kind = RegisterKind::Nonparametric;
                reg.size = variable.compile_time_value;
            }
        }
    } else {
        // single qubit
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


std::any ProgramCollector::visitOldStyleDeclarationStatement(
    qasm3Parser::OldStyleDeclarationStatementContext *ctx) {

    if (current_gate != nullptr) {
        throw std::runtime_error(
            "Register declarations are not allowed inside gate bodies");
    }

    RegisterType reg_type;
    if (ctx->QREG()) {
        reg_type = RegisterType::Qubit;
    } else if (ctx->CREG()) {
        reg_type = RegisterType::Int;  // Classical bits stored as Int
    } else {
        throw std::runtime_error("Unknown register type in old-style declaration");
    }

    std::string reg_name = ctx->Identifier()->getText();

    RegisterDef reg;
    reg.name = reg_name;
    reg.type = reg_type;

    auto designator = ctx->designator();
    if (designator) {
        auto expr = designator->expression();
        
        // Try to extract as literal integer
        if (auto size = parse_utils::tryExtractIntConst(expr)) {
            reg.kind = RegisterKind::Nonparametric;
            reg.size = expr->getText();
        } else {
            // Try to resolve as constant variable
            std::string expr_text = expr->getText();
            auto* sym = _scopes.lookupSymbol(expr_text);

            if (!sym || sym->kind != SymbolKind::ConstVar) {
                throw std::runtime_error(
                    "Cannot determine constant value of register size from \""
                    + expr_text + "\"");
            }

            // Look up variable to get compile-time value
            auto variable = _ir.getGlobalVariable(expr_text);
            if (variable.compile_time_value.empty()) {
                reg.kind = RegisterKind::Parametric;
                reg.size = expr_text;
            } else {
                reg.kind = RegisterKind::Nonparametric;
                reg.size = variable.compile_time_value;
            }
        }
    } else {
        reg.kind = RegisterKind::Nonparametric;
        reg.size = "1";
    }

    auto id = _ir.addRegister(reg);

    _scopes.addSymbol(Symbol{
        .name = reg_name,
        .kind = SymbolKind::Register,
        .ir_ref = id,
    });

    return nullptr;
}



/** EOF RegisterCollector.cpp */