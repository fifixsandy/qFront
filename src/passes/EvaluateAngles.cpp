/**
 * @file EvaluateAngles.cpp
 * @brief Pass that evaluates all gate parameter expressions to double literals.
 */

#include "Passes.hpp"
#include <exprtk.hpp>
#include <stdexcept>
#include <cstdio>

static long double evaluate_angle(const std::string& expr_str) {
    exprtk::symbol_table<long double> symbols;
    symbols.add_constants();

    exprtk::expression<long double> expr;
    expr.register_symbol_table(symbols);

    exprtk::parser<long double> parser;
    if (!parser.compile(expr_str, expr)) {
        throw std::runtime_error(
            "EvaluateAngles: failed to parse angle expression: " + expr_str);
    }

    return expr.value();
}

static std::string angle_to_string(long double val) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.17Lg", val);
    return std::string(buf);
}

static void evaluateBlock(std::vector<ProgramNodePtr>& body) {
    for (auto& node_ptr : body) {
        if (auto* gate_app = dynamic_cast<GateApplication*>(node_ptr.get())) {
            for (auto& param : gate_app->params) {
                long double val = evaluate_angle(param);
                param = angle_to_string(val);
            }
        } else if (auto* loop = dynamic_cast<LoopApplication*>(node_ptr.get())) {
            evaluateBlock(loop->body.body);
        } else if (auto* cond = dynamic_cast<ConditionalApplication*>(node_ptr.get())) {
            evaluateBlock(cond->then_body);
            evaluateBlock(cond->else_body);
        }
    }
}

void passes::evaluateAngles(IR& ir) {
    evaluateBlock(ir.getGlobalBlock().body);
}