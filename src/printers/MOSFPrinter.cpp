/**
 * @file MOSFPrinter.cpp
 * @author Filip Novak
 * @date 2026-04-28
 */

#include "../../inc/ir.hpp"
#include "../../inc/printers/MOSFPrinter.hpp"
#include <exprtk.hpp>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

using ordered_json = nlohmann::ordered_json;



static double parse_angle_expr(const std::string& s) {
    exprtk::symbol_table<double> symbols;
    symbols.add_constants();

    exprtk::expression<double> expr;
    expr.register_symbol_table(symbols);

    exprtk::parser<double> parser;
    if (!parser.compile(s, expr)) {
        throw std::runtime_error("Failed to parse angle expression: " + s);
    }

    return expr.value();
}


std::string MOSFPrinter::qubitVarName(const RegisterRef &ref, const IR &ir) const {
    const auto& reg = ir.getRegister(ref.reg_id);
    return reg.name + "[" + ref.qubit_index + "]";
}
void MOSFPrinter::assignLevels(const IR& ir) {
    this->qubit_levels.clear();

    int assigned_level = 0;
    for (const auto& reg : ir.getAllRegisters()) {
        if (reg.type != RegisterType::Qubit) continue;
        for (size_t i = 0; i < std::stoul(reg.size); ++i) {
            std::string var_name = reg.name + "[" + std::to_string(i) + "]";
            qubit_levels[var_name] = assigned_level++;
        }
    }
}

ordered_json MOSFPrinter::buildVars() const {
    ordered_json vars_json = ordered_json::object();

    std::vector<std::pair<std::string, int>> sorted(qubit_levels.begin(), qubit_levels.end());
    std::sort(sorted.begin(), sorted.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });

    for (const auto& [name, level] : sorted)
        vars_json[name] = level;

    return vars_json;
}

ordered_json MOSFPrinter::buildDefs() const {
    ordered_json defs_json = ordered_json::object();

    if (needs_high_swap) {
        defs_json["high_swap"] = ordered_json{
            {"put_up", ordered_json{
                {"type", "get_side"},
                {"side", "R"}
            }},
            {"put_in", ordered_json{
                {"type", "makenode"},
                {"low", ordered_json{
                    {"type", "get_side"},
                    {"side", "L"}
                }},
                {"high", ordered_json{
                    {"type", "received"}
                }}
            }}
        };
    }

    return defs_json;
}

ordered_json MOSFPrinter::emitMCX(GateApplication app, const IR& ir) {
    // Convention: last operand = target, preceding = controls
    if (app.operands.size() < 1)
        throw std::runtime_error("MOSFPrinter: [M(C)]X gate expects at least 1 operand.");

    std::vector<std::string> ctrl_vars;
    for (size_t i = 0; i + 1 < app.operands.size(); ++i)
        ctrl_vars.push_back(qubitVarName(app.operands[i], ir));
    const std::string tgt_var = qubitVarName(app.operands.back(), ir);

    const int xt = qubit_levels.at(tgt_var);

    std::vector<std::pair<int, std::string>> above_t, below_t;
    for (const auto& ctrl : ctrl_vars) {
        const int xc = qubit_levels.at(ctrl);
        if      (xc < xt) above_t.push_back({xc, ctrl});
        else if (xc > xt) below_t.push_back({xc, ctrl});
        else throw std::runtime_error(
            "MOSFPrinter: control and target at same BDD level.");
    }
    std::sort(above_t.begin(), above_t.end()); // shallowest first
    std::sort(below_t.begin(), below_t.end()); // shallowest first

    // --- Action at target node ---
    // Controls below target (xc > xt): lockstep down to each,
    // swapping only HIGH children --> mirrors high_swap_param + mtbdd_make_swap
    ordered_json at_target;
    if (below_t.empty()) {
        at_target = {{"type", "swap"}};
    } else {
        needs_high_swap = true; // set global flag to emit high_swap definition
        // Build innermost-first (deepest control first)
        ordered_json inner = {
            {"type",   "swap"},
            {"paramL", {{"$ref", "high_swap"}}},
            {"paramR", {{"$ref", "high_swap"}}}
        };
        for (auto it = below_t.rbegin(); it != below_t.rend(); ++it) {
            inner = {
                {"type",        "lockstep_to"},
                {"target",      {{"var", it->second}}},
                {"action",      inner},
                {"pref_L",      "LR"},
                {"action_on_L", "R"},
                {"pref_R",      "LR"},
                {"action_on_R", "R"}
            };
        }
        at_target = inner;
    }

    // Wrap with traverse_to target level
    ordered_json res = {
        {"type",   "traverse_to"},
        {"target", {{"var", tgt_var}}},
        {"action", at_target}
    };

    // Controls above target (xc < xt): traverse to each, HIGH branch only
    // (control = 1) --> mirrors Branch::R / action_on = "R"
    for (auto it = above_t.rbegin(); it != above_t.rend(); ++it) {
        res = {
            {"type",      "traverse_to"},
            {"target",    {{"var", it->second}}},
            {"action_on", "R"},
            {"action",    res}
        };
    }

    return res;
}

ordered_json MOSFPrinter::emitH(GateApplication app, const IR& ir) {
    if (app.operands.size() != 1)
        throw std::runtime_error("MOSFPrinter: H gate expects exactly 1 operand.");

    const std::string tgt_var = qubitVarName(app.operands[0], ir);

    return ordered_json{
        {"type",   "traverse_to"},
        {"target", {{"var", tgt_var}}},
        {"action", ordered_json{
            {"type", "makenode"},
            {"low",  ordered_json{
                {"type", "plus_mulsqrt2"},
                {"L",    {{"type", "get_side"}, {"side", "L"}}},
                {"R",    {{"type", "get_side"}, {"side", "R"}}}
            }},
            {"high", ordered_json{
                {"type", "minus_mulsqrt2"},
                {"L",    {{"type", "get_side"}, {"side", "L"}}},
                {"R",    {{"type", "get_side"}, {"side", "R"}}}
            }}
        }},
        {"x_1_sqrt(2)", 1} // hint to multiply by 1/sqrt(2) for normalization; not part of core MOSF semantics
    };
}

ordered_json MOSFPrinter::emitRX(GateApplication app, const IR& ir) {
    if (app.operands.size() != 1)
        throw std::runtime_error("MOSFPrinter: RX gate expects exactly 1 operand.");
    if (app.params.size() != 1)
        throw std::runtime_error("MOSFPrinter: RX gate expects exactly 1 parameter (angle).");

    const std::string tgt_var = qubitVarName(app.operands[0], ir);
    const std::string angle = app.params[0];
    double angle_val = parse_angle_expr(angle);
    return ordered_json{
        {"type",   "traverse_to"},
        {"target", {{"var", tgt_var}}},
        {"action", ordered_json{
            {"type", "rx"},
            {"angle", angle_val},
        }
        }
    };
}

ordered_json MOSFPrinter::emitRY(GateApplication app, const IR& ir) {
    if (app.operands.size() != 1)
        throw std::runtime_error("MOSFPrinter: RY gate expects exactly 1 operand.");
    if (app.params.size() != 1)
        throw std::runtime_error("MOSFPrinter: RY gate expects exactly 1 parameter (angle).");

    const std::string tgt_var = qubitVarName(app.operands[0], ir);
    const std::string angle = app.params[0];
    double angle_val = parse_angle_expr(angle);

    return ordered_json{
        {"type",   "traverse_to"},
        {"target", {{"var", tgt_var}}},
        {"action", ordered_json{
            {"type", "ry"},
            {"angle", angle_val}}
        }
    };
}

ordered_json MOSFPrinter::emitZ(GateApplication app, const IR& ir) {
    if (app.operands.size() != 1)
        throw std::runtime_error("MOSFPrinter: Z gate expects exactly 1 operand.");

    const std::string tgt_var = qubitVarName(app.operands[0], ir);

    return ordered_json{
        {"type",   "traverse_to"},
        {"target", {{"var", tgt_var}}},
        {"action", ordered_json{
            {"type", "makenode"},
            {"low", ordered_json{
                {"type", "get_side"},
                {"side", "L"}
            }},
            {"high", ordered_json{
                {"type", "neg"},
                {"child", ordered_json{
                    {"type", "get_side"},
                    {"side", "R"}
                }}
            }},
            {"x_gate_name", "z"}
        }}
    };
}

ordered_json MOSFPrinter::emitS(GateApplication app, const IR& ir) {
    if (app.operands.size() != 1)
        throw std::runtime_error("MOSFPrinter: S gate expects exactly 1 operand.");

    const std::string tgt_var = qubitVarName(app.operands[0], ir);

    return ordered_json{
        {"type",   "traverse_to"},
        {"target", {{"var", tgt_var}}},
        {"action", ordered_json{
            {"type", "makenode"},
            {"low", ordered_json{
                {"type", "get_side"},
                {"side", "L"}
            }},
            {"high", ordered_json{
                {"type", "i_mul"},
                {"child", ordered_json{
                    {"type", "get_side"},
                    {"side", "R"}
                }}
            }},
            {"x_gate_name", "s"}
        }}
    };
}

ordered_json MOSFPrinter::emitY(GateApplication app, const IR& ir) {
    if (app.operands.size() != 1)
        throw std::runtime_error("MOSFPrinter: Y gate expects exactly 1 operand.");

    const std::string tgt_var = qubitVarName(app.operands[0], ir);

    return ordered_json{
        {"type",   "traverse_to"},
        {"target", {{"var", tgt_var}}},
        {"action", ordered_json{
            {"type", "makenode"},
            {"low", ordered_json{
                {"type", "i_mul"},
                {"child", ordered_json{
                    {"type", "get_side"},
                    {"side", "R"}
                }}
            }},
            {"high", ordered_json{
                {"type", "neg_i_mul"},
                {"child", ordered_json{
                    {"type", "get_side"},
                    {"side", "L"}
                }}
            }},
            {"x_gate_name", "y"}
        }}
    };
}

ordered_json MOSFPrinter::emitCZ(GateApplication app, const IR& ir) {
    if (app.operands.size() != 2)
        throw std::runtime_error("MOSFPrinter: CZ gate expects exactly 2 operands (control, target).");

    std::string ctrl_var = qubitVarName(app.operands[0], ir);
    std::string tgt_var  = qubitVarName(app.operands[1], ir);

    if (qubit_levels.at(ctrl_var) > qubit_levels.at(tgt_var)) {
        ctrl_var.swap(tgt_var);
    }

    return ordered_json{
        {"type",      "traverse_to"},
        {"target",    {{"var", ctrl_var}}},
        {"action_on", "R"},  // control=1 (HIGH branch only)
        {"action", ordered_json{
            {"type",   "traverse_to"},
            {"target", {{"var", tgt_var}}},
            {"action", ordered_json{
                {"type", "makenode"},
                {"low", ordered_json{
                    {"type", "get_side"},
                    {"side", "L"}
                }},
                {"high", ordered_json{
                    {"type", "neg"},
                    {"child", ordered_json{
                        {"type", "get_side"},
                        {"side", "R"}
                    }}
                }}
            }}
        }},
        {"x_gate_name", "cz"}
    };
}

ordered_json MOSFPrinter::emitTdg(GateApplication app, const IR& ir) {
    if (app.operands.size() != 1)
        throw std::runtime_error("MOSFPrinter: Tdg gate expects exactly 1 operand.");

    const std::string tgt_var = qubitVarName(app.operands[0], ir);
    auto phase_expr = "-pi/4"; // e^(-iπ/4)
    double phase_val = parse_angle_expr(phase_expr);
    return ordered_json{
        {"type",   "traverse_to"},
        {"target", {{"var", tgt_var}}},
        {"action", ordered_json{
            {"type", "makenode"},
            {"low", ordered_json{
                {"type", "get_side"},
                {"side", "L"}
            }},
            {"high", ordered_json{
                {"type", "phase_mul"},
                {"child", ordered_json{
                    {"type", "get_side"},
                    {"side", "R"}
                }},
                {"phase", phase_val}  // e^(-iπ/4)
            }},
            {"x_gate_name", "tdg"}
        }}
    };
}

ordered_json MOSFPrinter::emitRZ(GateApplication app, const IR& ir) {
    if (app.operands.size() != 1)
        throw std::runtime_error("MOSFPrinter: RY gate expects exactly 1 operand.");
    if (app.params.size() != 1)
        throw std::runtime_error("MOSFPrinter: RY gate expects exactly 1 parameter (angle).");

    const std::string tgt_var = qubitVarName(app.operands[0], ir);
    const std::string angle = app.params[0];
    double angle_val = parse_angle_expr(angle);
    return ordered_json{
        {"type",   "traverse_to"},
        {"target", {{"var", tgt_var}}},
        {"action", ordered_json{
            {"type", "rz"},
            {"angle", angle_val}}
        }
    };
}

ordered_json MOSFPrinter::emitT(GateApplication app, const IR& ir) {
    if (app.operands.size() != 1)
        throw std::runtime_error("MOSFPrinter: T gate expects exactly 1 operand.");

    const std::string tgt_var = qubitVarName(app.operands[0], ir);
    auto phase_expr = "pi/4"; // e^(iπ/4)
    double phase_val = parse_angle_expr(phase_expr);
    return ordered_json{
        {"type",   "traverse_to"},
        {"target", {{"var", tgt_var}}},
        {"action", ordered_json{
            {"type", "makenode"},
            {"low", ordered_json{
                {"type", "get_side"},
                {"side", "L"}
            }},
            {"high", ordered_json{
                {"type", "phase_mul"},
                {"child", ordered_json{
                    {"type", "get_side"},
                    {"side", "R"}
                }},
                {"phase", phase_val}  // e^(iπ/4)
            }},
            {"x_gate_name", "t"}
        }}
    };
}

ordered_json MOSFPrinter::dispatchGate(GateApplication app, const IR &ir) {
    const auto& gate = ir.getGate(app.gate_id);
    if (gate.name == "x" || gate.name == "cx" ||
        gate.name == "ccx" || gate.name == "mcx") {
        return emitMCX(app, ir);
    } else if (gate.name == "h") {
        return emitH(app, ir);
    } else if (gate.name == "rx") {
        return emitRX(app, ir);
    } else if (gate.name == "ry") {
        return emitRY(app, ir);
    } else if (gate.name == "rz") {
        return emitRZ(app, ir);
    } else if (gate.name == "z") {
        return emitZ(app, ir);
    } else if (gate.name == "s") {
        return emitS(app, ir);
    } else if (gate.name == "y") {
        return emitY(app, ir);
    } else if (gate.name == "cz") {
        return emitCZ(app, ir);
    } else if (gate.name == "tdg") {
        return emitTdg(app, ir);
    } else if (gate.name == "t") {
        return emitT(app, ir);
    } else {    
        throw std::runtime_error("MOSFPrinter: unsupported gate: " + gate.name);
    }
}

ordered_json MOSFPrinter::dispatchLoop(const LoopApplication& loop, const IR& ir) {
    // Resolve static iteration count; -1 means symbolic/unknown
    int repeat = ir.resolveLoopCount(loop.values);

    ordered_json inner_ops = ordered_json::array();
    for (const auto& node : loop.body.body) {
        if (auto* ga = dynamic_cast<const GateApplication*>(node.get())) {
            inner_ops.push_back(dispatchGate(*ga, ir));
        } else if (auto* inner_loop = dynamic_cast<const LoopApplication*>(node.get())) {
            inner_ops.push_back(dispatchLoop(*inner_loop, ir));
        } else {
            throw std::runtime_error(
                "MOSFPrinter: unsupported node type inside loop body.");
        }
    }

    ordered_json group;
    group["type"] = "group";

    // Use loop variable name as group name if available, else generic label
    if (std::holds_alternative<std::string>(loop.values))
        group["name"] = std::get<std::string>(loop.values);
    else
        group["name"] = "loop_" + std::to_string(next_group_id++);

    if (repeat > 0)
        group["repeat"] = repeat;
    else
        std::cerr << "[warning] symbolic loop repeat count, omitting 'repeat' field\n";

    group["ops"] = inner_ops;
    return group;
}

// ordered_json MOSFPrinter::dispatchCond(ConditionalApplication cond, const IR& ir) {
//     // Placeholder for conditional application dispatch; not implemented
//     throw std::runtime_error("MOSFPrinter: conditional applications not yet supported.");
// }

ordered_json MOSFPrinter::buildOps(const IR& ir) {
    ordered_json ops_json = ordered_json::array();
    for (auto &node : ir.getGlobalBlock().body) {
        if (auto* gate_app = dynamic_cast<GateApplication*>(node.get())){
                ops_json.push_back(dispatchGate(*gate_app, ir));
        } else if (auto* loop_app = dynamic_cast<LoopApplication*>(node.get())) {
            ops_json.push_back(dispatchLoop(*loop_app, ir));
        } else if (auto* cond_app = dynamic_cast<ConditionalApplication*>(node.get())) {
            //ops_json.push_back(dispatchCond(*cond_app, ir));
            throw std::runtime_error("MOSFPrinter: conditional applications not yet supported.");
        } else {
            throw std::runtime_error("MOSFPrinter: unknown ProgramNode type during ops generation");
        }
    }

    return ops_json;
}

void MOSFPrinter::print(const IR& ir, std::ostream& out) {
    assignLevels(ir);

    ordered_json mosf_json;
    mosf_json["version"] = mosf_version;
    mosf_json["tree_id"] = tree_id;
    mosf_json["vars"] = buildVars();
    mosf_json["x_levels"] = qubit_levels.size();
    mosf_json["defs"] = buildDefs();
    mosf_json["ops"]  = buildOps(ir);

    out << mosf_json.dump(4) << std::endl; // pretty-print with 4-space indent
}

/* EOF MOSFPrinter.cpp */