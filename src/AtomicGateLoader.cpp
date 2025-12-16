#include "../inc/AtomicGateLoader.hpp"

std::vector<GateDef> loadGates(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open JSON file: " + filename);
    }

    json j;
    file >> j;

    std::vector<GateDef> gates;
    for (const auto& g : j["gates"]) {
        GateDef def;
        const auto& names = g["names"];

        def.name = names.at(0).get<std::string>();  // canonical
        for (size_t i = 1; i < names.size(); ++i) {
            def.aliases.push_back(names[i].get<std::string>());
        }
        def.kind = GateKind::Atomic;
        AtomicGateSemantics sem;
        sem.matrix = g["matrix"].dump();
        def.semantics = sem;

        gates.push_back(std::move(def));
    }
    return gates;
}