#include "Passes.hpp"
#include "merge.hpp"

/**
 * Recursively rewrites all RegisterRefs in a block.
 * Handles GateApplication, LoopApplication, and ConditionalApplication.
 *
 * @param body          Block body to rewrite in-place.
 * @param offset_map    Map from old reg_id to offset.
 * @param merged_id     The id of the new merged register.
 */
static void rewriteRegistersRefsInBlock(
    std::vector<ProgramNodePtr>& body,
    const std::unordered_map<idRegister, std::size_t>& offset_map,
    idRegister merged_id
) {
    for (auto& node_ptr : body) {
        if (auto* gate_app = dynamic_cast<GateApplication*>(node_ptr.get())) {
            for (auto& op : gate_app->operands) {
                rewriteRef(op, offset_map, merged_id);
            }
        } else if (auto* loop = dynamic_cast<LoopApplication*>(node_ptr.get())) {
            rewriteRegistersRefsInBlock(loop->body.body, offset_map, merged_id);
        } else if (auto* cond = dynamic_cast<ConditionalApplication*>(node_ptr.get())) {
            rewriteRegistersRefsInBlock(cond->then_body, offset_map, merged_id);
            rewriteRegistersRefsInBlock(cond->else_body, offset_map, merged_id);
        }
    }
}


void passes::mergeRegisters(IR& ir) {
    auto mergeable_regs = collectMergeableRegisters(ir);
    if (mergeable_regs.size() <= 1) return;

    auto offset_map = computeOffsets(mergeable_regs);

    // Total size
    std::size_t total_size = 0;
    for (const auto& [_, size] : mergeable_regs)
        total_size += size;

    // Repurpose the FIRST register as the merged one -id stays stable
    idRegister merged_id = mergeable_regs[0].first;
    RegisterDef& merged = ir.getRegister(merged_id);
    merged.name = "__merged_qubits";
    merged.size = std::to_string(total_size);

    // Rewrite all refs (including refs that already point to merged_id - offset is 0, no-op)
    rewriteRegistersRefsInBlock(ir.getGlobalBlock().body, offset_map, merged_id);

    // Zero out the remaining registers - do NOT remove (would shift ids),
    // during emitting, registers with size "0" are skipped
    for (std::size_t i = 1; i < mergeable_regs.size(); ++i) {
        RegisterDef& reg = ir.getRegister(mergeable_regs[i].first);
        reg.size = "0";
        reg.name = "__unused";
    }
}