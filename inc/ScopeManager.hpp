/**
 * @file ScopeManager.hpp
 * @author Filip Novak
 * @date 2025-12-20
 * 
 * Semantics from: https://openqasm.com/language/scope.html
 */
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <variant>
#include "ir.hpp"

enum class ScopeKind {
    Global,
    GateOrSubroutine,
    Block
};

enum class SymbolKind {
    Gate,
    Subroutine,
    ConstVar,
    Register,
    Var,
    Alias,
    Builtin,
    Qubit,
    Parameter
};

struct Symbol {
    std::string name;
    SymbolKind kind;

    std::variant<std::monostate, size_t, std::string> ir_ref;

    std::vector<std::string> aliases;
};

struct Scope {
    ScopeKind kind;
    std::unordered_map<std::string, Symbol> symbols;
};

class ScopeManager {

public:
    ScopeManager();

    void enterScope(ScopeKind kind);
    void exitScope();

    ScopeKind currentScopeKind(void) const;

    void addSymbol(const Symbol& symbol);

    const Symbol* lookupSymbol(const std::string& name) const;

private:
    std::vector<Scope> _scopes;

    bool isVisibleFromGateOrSubroutineScope(const Symbol& sym) const;

};

