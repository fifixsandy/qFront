/**
 * @file ScopeManager.cpp
 * @author Filip Novak
 * @date 2025-12-20
 */


#include "ScopeManager.hpp"
#include <stdexcept>
#include <iostream>

ScopeManager::ScopeManager() {
    enterScope(ScopeKind::Global);
}

void ScopeManager::enterScope(ScopeKind kind) {
    Scope scope;
    scope.kind = kind;
    _scopes.push_back(scope);
}

void ScopeManager::exitScope() {
    if (_scopes.empty()) {
        throw std::logic_error("Exiting scope, but no scope is active");
    }

    if (_scopes.back().kind == ScopeKind::Global) {
        throw std::logic_error("Exiting global scope");
    }

    _scopes.pop_back();
}

void ScopeManager::addSymbol(const Symbol& symbol) {
    if (_scopes.empty()) {
        throw std::logic_error("No active scope to define symbol");
    }

    auto& current_scope = _scopes.back();

    // check canonical name
    if (current_scope.symbols.count(symbol.name)) {
        throw std::runtime_error("Symbol already defined in current scope: " + symbol.name);
    }

    // canonical symbol
    current_scope.symbols[symbol.name] = symbol;

    // if aliases, add them pointing to the same symbol
    for (const auto& alias : symbol.aliases) {
        if (current_scope.symbols.count(alias)) {
            throw std::runtime_error("Alias already defined in current scope: " + alias);
        }

        Symbol alias_symbol = symbol;
        alias_symbol.name = alias;
        current_scope.symbols[alias] = alias_symbol;
    }
}


const Symbol* ScopeManager::lookupSymbol(const std::string& name) const {
    bool fromGateOrSubroutine = false;
    for (auto it = _scopes.rbegin(); it != _scopes.rend(); ++it) { // check each scope
        if (it->kind == ScopeKind::GateOrSubroutine) // in gate def/subroutine
            fromGateOrSubroutine = true;

        auto found = it->symbols.find(name);
        
        if (found == it->symbols.end()) // not found in current scope
            continue;

        // Found symbol in same or inner scope : always visible
        if (!fromGateOrSubroutine || it->kind != ScopeKind::Global)
            return &found->second;

        // Found in global, but lookup originates from gate/subroutine
        // only some symbols are visible from gatedef/subroutine, when in global
        // scope. refer to https://openqasm.com/language/scope.html#subroutine-and-gate-scope
        if (isVisibleFromGateOrSubroutineScope(found->second))
            return &found->second;
    }
    return nullptr;
}

bool ScopeManager::isVisibleFromGateOrSubroutineScope(const Symbol& sym) const {
    switch (sym.kind) {
        case SymbolKind::ConstVar:
        case SymbolKind::Gate:
        case SymbolKind::Subroutine:
        case SymbolKind::Builtin:
            return true;

        default:
            return false;
    }
}