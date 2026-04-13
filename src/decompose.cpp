/**
 * @file decompose.cpp
 * @author Filip Novak
 */
#include "decompose.hpp"
#include "ir.hpp"
#include <cassert>
#include <stdexcept>

static GateApplication makeGateApp(idGate gate_id, std::vector<RegisterRef> operands) {
    GateApplication app;
    app.gate_id  = gate_id;
    app.operands = std::move(operands);
    return app;
}

std::vector<GateApplication> buildMCXChain(
    const GateApplication& mcx,
    const std::vector<RegisterRef>& ancillas,
    IR& ir
) {
    std::vector<GateApplication> chain;

    // Operand layout: [ctrl_0, ..., ctrl_{n-1}, target]
    const auto& ops      = mcx.operands;
    const std::size_t n  = ops.size();
    assert(n >= 1 && "MCX must have at least a target operand");

    const std::size_t n_controls = n - 1;
    const RegisterRef& target = ops.back();
    const idGate id_x = ir.getGateId("x");
    const idGate id_cx = ir.getGateId("cx");
    const idGate id_ccx = ir.getGateId("ccx");

    if (n_controls == 0) {
        // X(target)
        chain.push_back(makeGateApp(id_x, {target}));
        ir.markGateUsed(id_x);

    } else if (n_controls == 1) {
        // CX(ctrl_0, target)
        chain.push_back(makeGateApp(id_cx, {ops[0], target}));
        ir.markGateUsed(id_cx);

    } else if (n_controls == 2) {
        // CCX(ctrl_0, ctrl_1, target)
        chain.push_back(makeGateApp(id_ccx, {ops[0], ops[1], target}));
        ir.markGateUsed(id_ccx);

    } else {
        // decompose into V-chain using ancillas
        assert(ancillas.size() >= n_controls - 2
               && "Not enough ancilla qubits for V-chain decomposition");

        chain.push_back(makeGateApp(id_ccx, 
                                {ops[0], ops[1], ancillas[0]}));

        for (std::size_t i = 1; i <= n_controls - 3; ++i) {
            chain.push_back(makeGateApp(id_ccx, 
                                    {ops[i + 1], ancillas[i - 1], ancillas[i]}));
        }

        chain.push_back(makeGateApp(id_ccx, 
                                {ops[n_controls - 1], 
                                ancillas[n_controls - 3], 
                                target}));

        // uncompute to reuse ancillas
        for (std::size_t i = n_controls - 3; i >= 1; --i) {
            chain.push_back(makeGateApp(id_ccx, {ops[i + 1], ancillas[i - 1], ancillas[i]}));
        }

        chain.push_back(makeGateApp(id_ccx, {ops[0], ops[1], ancillas[0]}));

    }

    return chain;
}