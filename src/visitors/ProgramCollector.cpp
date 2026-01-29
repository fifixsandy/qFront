/**
 * @file ProgramCollector.cpp
 * @author Filip Novak
 * @date 2025-12-27
 */

#include "../../inc/visitors/ProgramCollector.hpp"
#include "../../inc/utils.hpp"

ProgramCollector::ProgramCollector(
    IR& ir, ScopeManager& scopes): _ir(ir), _scopes(scopes) {
    block_stack.push_back(&ir.getGlobalBlock());
}

std::any ProgramCollector::inProgram_visitGateCallStatement(
    qasm3Parser::GateCallStatementContext* ctx) {
    auto application = std::make_unique<GateApplication>();

    const std::string gate_name = ctx->Identifier()->getText();
    auto* sym = _scopes.lookupSymbol(gate_name);

    if (!sym || sym->kind != SymbolKind::Gate) {
        throw std::runtime_error("Unknown gate: " + gate_name);
    }

    application->gate_id = std::get<size_t>(sym->ir_ref);
    _ir.markGateUsed(application->gate_id);

    auto operandCtxs = ctx->gateOperandList()->gateOperand();

    for (auto* operandCtx : operandCtxs) {
        auto* indexed = operandCtx->indexedIdentifier();
        if (!indexed) {
            throw std::runtime_error("Hardware qubits not supported yet");
        }

        const std::string operandName =
            indexed->Identifier()->getText();

        auto* regSym = _scopes.lookupSymbol(operandName);
        if (!regSym || regSym->kind != SymbolKind::Register) {
            throw std::runtime_error("Unknown register: " + operandName);
        }

        RegisterRef ref;
        ref.reg_id = std::get<size_t>(regSym->ir_ref);

        if (!indexed->indexOperator().empty()) {
            auto* indexOp = indexed->indexOperator(0);
            auto exprs = indexOp->expression();

            if (exprs.empty()) {
                throw std::runtime_error("Index operator has no expression");
            }

            // TODO: replace with ExprPtr
            ref.qubit_index = exprs[0]->getText();
        } else {
            ref.qubit_index = "0";
        }

        application->operands.push_back(std::move(ref));
    }

    block_stack.back()->body.push_back(std::move(application));
    return nullptr;
}

std::any ProgramCollector::inProgram_visitForStatement(
    qasm3Parser::ForStatementContext *ctx) {
    _scopes.enterScope(ScopeKind::Block);

    auto loop = std::make_unique<LoopApplication>();

    // Parse loop variable type
    loop->type = TypeExpr{
        .base = ctx->scalarType()->getText(),
        .dims = {},
        .is_const = true
    };

    loop->variable = ctx->Identifier()->getText();

    // Declare loop variable in IR block
    VariableDef loopVar {
        .name = loop->variable,
        .type = loop->type,
        .is_const = true,
        .initializer = "" // loop variable has no initializer todo change to expr
    };
    loop->body.variables.push_back(std::move(loopVar));

    // Register symbol for semantic checks
    _scopes.addSymbol(Symbol{
        .name = loop->variable,
        .kind = SymbolKind::Var,
    });

    // Parse iteration domain
    if (auto* range = ctx->rangeExpression()) {
        Interval interval;

        auto exprs = range->expression();
        interval.start = exprs.front()->getText();
        interval.end   = exprs.back()->getText();

        if (exprs.size() == 3) {
            interval.step = exprs[1]->getText();
        }

        loop->values = interval;
    }
    else if (auto* set = ctx->setExpression()) {
        std::vector<std::string> values;
        for (auto* expr : set->expression()) {
            values.push_back(expr->getText());
        }
        loop->values = values;
    }
    else {
        // for T x in expr
        loop->values = ctx->expression()->getText();
    }

    // Visit loop body
    block_stack.push_back(&loop->body);
    visit(ctx->statementOrScope());
    block_stack.pop_back();

    // Attach loop to parent block
    block_stack.back()->body.push_back(std::move(loop));

    _scopes.exitScope();
    return nullptr;
}

std::any ProgramCollector::visitConstDeclarationStatement(
    qasm3Parser::ConstDeclarationStatementContext *ctx) {
    
    // Extract name
    std::string name = ctx->Identifier()->getText();

    // Extract type
    std::string type = ctx->scalarType()->getText();

    // Extract initializer (could be __nondet_uint() or a literal)
    std::string initializer;
    if (ctx->declarationExpression() != nullptr) {
        initializer = ctx->declarationExpression()->getText();
    }

    std::string compile_time_value;
    if (ctx->declarationExpression()) {
        auto* expr = ctx->declarationExpression()->expression();
        if (expr) {
            if (auto val = parse_utils::tryExtractIntConst(expr)) {
                compile_time_value = std::to_string(*val);
            }
        }
    }

    // Create IR variable
    VariableDef var;
    var.name = name;
    var.type.base = type;
    var.is_const = true;
    var.compile_time_value = compile_time_value;
    var.initializer = initializer;

    block_stack.back()->variables.push_back(var);
    _scopes.addSymbol(Symbol{
        .name = name,
        .kind = SymbolKind::ConstVar,
    });

    return nullptr;
};


std::any ProgramCollector::visitGateStatement(
    qasm3Parser::GateStatementContext *ctx) {
    current_gate = &_ir.getGate(ctx->Identifier()->getText());
    _scopes.enterScope(ScopeKind::GateOrSubroutine);
    
    // adding qubit argument identifiers to current scope - they can shadow
    // symbols from outer scopes
    for (const auto& a : current_gate->argument_qubits) {
        _scopes.addSymbol(Symbol{
            .name = a,
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

    auto& top_body = std::get<CompositeGateBody>(current_gate->semantics).body;
    body_stack.push_back(&top_body);

    visit(ctx->scope());

    body_stack.pop_back();

    // leaving, reset scopes
    current_gate = nullptr;
    _scopes.exitScope();
    return nullptr;
}

std::any ProgramCollector::visitDefStatement(
    qasm3Parser::DefStatementContext* ctx) {
    
    auto& subroutine = _ir.getSubroutine(ctx->Identifier()->getText());

    // Process body
    _scopes.enterScope(ScopeKind::GateOrSubroutine);
    block_stack.push_back(&subroutine.body);

    // adding parameter identifiers to current scope
    for (const auto& p : subroutine.parameters) {
        _scopes.addSymbol(Symbol{
            .name = p.name,
            .kind = SymbolKind::Parameter,
        });
    }

    visit(ctx->scope());

    block_stack.pop_back();
    _scopes.exitScope();

    return nullptr;
}

std::any ProgramCollector::visitGateCallStatement(qasm3Parser::GateCallStatementContext* ctx) {
    if (current_gate) {
        return gateBody_visitGateCallStatement(ctx);
    } else {
        return inProgram_visitGateCallStatement(ctx);
    }
    return nullptr;
}
    
std::any ProgramCollector::visitForStatement(qasm3Parser::ForStatementContext *ctx) {
    _scopes.enterScope(ScopeKind::Block);
    if (current_gate) {
        gateBody_visitForStatement(ctx);
    } else {
        inProgram_visitForStatement(ctx);
    }
    _scopes.exitScope();
    return nullptr;
}
/* EOF ProgramCollector.cpp */