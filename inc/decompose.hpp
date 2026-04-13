#pragma once
#include "ir.hpp"

/**
 * Builds a chain of GateApplications that implements the given MCX gate using
 * the standard V-chain decomposition. The input MCX application must have the
 * control and target operands in the order [ctrl_0, ..., ctrl_{n-1}, target].
 * The ancillas vector must contain at least n-3 ancilla qubits for n >= 3.
 * 
 * @param mcx The MCX gate application to decompose
 * @param ancillas A vector of RegisterRefs for ancilla qubits to use in the decomposition
 * @param ir The IR context to resolve gate IDs
 * 
 * @return A vector of GateApplications that implement the same operation as the input MCX
 *         using only X, CX, and CCX gates.
 */
std::vector<GateApplication> buildMCXChain(
    const GateApplication& mcx,
    const std::vector<RegisterRef>& ancillas,
    IR& ir
);