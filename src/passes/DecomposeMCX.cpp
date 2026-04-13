#include "Passes.hpp"
#include "decompose.hpp"

/**
 * @brief Decomposes all MCX gate applications in the block into chains of X, CX, and CCX gates.
 * 
 * This function recursively traverses the program nodes in the block, looking for GateApplications
 * that correspond to the MCX gate. When it finds one, it uses the buildMCXChain function to generate
 * a sequence of GateApplications that implement the same operation using only X, CX, and CCX gates.
 * It also keeps track of the number of ancilla qubits needed for the decomposition and updates the IR accordingly.
 * The function handles nested LoopApplication and ConditionalApplication nodes by recursively processing their bodies.
 * 
 * @param body The vector of ProgramNodePtr representing the body of a block to process
 * @param mcx_id The idGate corresponding to the MCX gate in the IR
 * @param ancillas A vector of RegisterRef that can be used as ancilla qubits for the decomposition (will be populated as needed)
 * @param necessary_ancillas A reference to an unsigned integer that will be updated with 
 *                           the maximum number of ancillas needed for any MCX decomposition
 * @param ancillas_register_id The idRegister of the register that will hold the ancilla qubits 
 *                             (must be added to the IR before calling this function)
 * @param ir The IR context to resolve gate and register information
 * 
 * @return A new vector of ProgramNodePtr with all MCX applications decomposed
 */
static std::vector<ProgramNodePtr> decomposeBlock(
    std::vector<ProgramNodePtr>& body,
    const idGate mcx_id,
    std::vector<RegisterRef>& ancillas,
    unsigned& necessary_ancillas,
    const idRegister ancillas_register_id,
    IR& ir
) {
    std::vector<ProgramNodePtr> new_body;
    for (auto& node_ptr : body) {
        if (auto* gate_app = dynamic_cast<GateApplication*>(node_ptr.get());
            gate_app && gate_app->gate_id == mcx_id) { // found an MCX application
            const auto n_controls = gate_app->operands.size() - 1;
            if (n_controls > 2) { // check if ancillas needed
                const unsigned needed = n_controls - 2;
                while (ancillas.size() < needed) {
                    ancillas.push_back(RegisterRef{
                        .reg_id      = ancillas_register_id,
                        .qubit_index = std::to_string(ancillas.size())
                    });
                }
                necessary_ancillas = std::max(necessary_ancillas, needed);
            }
            auto chain = buildMCXChain(*gate_app, ancillas, ir);
            for (auto& app : chain)
                new_body.push_back(std::make_unique<GateApplication>(std::move(app)));

        } else if (auto* loop = dynamic_cast<LoopApplication*>(node_ptr.get())) { // recursively decompose inside loops
            loop->body.body = decomposeBlock(
                loop->body.body, mcx_id, ancillas, necessary_ancillas, ancillas_register_id, ir);
            new_body.push_back(std::move(node_ptr));

        } else if (auto* cond = dynamic_cast<ConditionalApplication*>(node_ptr.get())) { // recursively decompose inside conditionals
            cond->then_body = decomposeBlock(
                cond->then_body, mcx_id, ancillas, necessary_ancillas, ancillas_register_id, ir);
            cond->else_body = decomposeBlock(
                cond->else_body, mcx_id, ancillas, necessary_ancillas, ancillas_register_id, ir);
            new_body.push_back(std::move(node_ptr));

        } else { // other nodes remain unchanged
            new_body.push_back(std::move(node_ptr));
        }
    }
    return new_body;
}

void passes::decomposeMCX(IR& ir) {
    if (!ir.hasGate("mcx") || !ir.getGate("mcx").used) return;

    const auto mcx_id = ir.getGateId("mcx");
    auto& global_block = ir.getGlobalBlock();

    RegisterDef ancillas_register{
        .name = "__ancillas_for_mcx",
        .kind = RegisterKind::Nonparametric,
        .type = RegisterType::Qubit,
        .size = "0" // will be updated later based on necessary ancillas
    };
    const auto ancillas_register_id = ir.addRegister(ancillas_register);

    unsigned necessary_ancillas = 0;
    std::vector<RegisterRef> ancillas;

    global_block.body = decomposeBlock(
        global_block.body, mcx_id, ancillas, necessary_ancillas, ancillas_register_id, ir);
    
    if (necessary_ancillas > 0) {
        ir.getRegister(ancillas_register_id).size = std::to_string(necessary_ancillas);
    } else {
        // no ancillas needed, remove the register
        ir.removeRegister(ancillas_register_id);
    }
    ir.markGateUnused(mcx_id);
}