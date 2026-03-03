#include "../inc/AtomicGateLoader.hpp"
std::vector<GateDef> loadGates(const std::string& filename,
                               bool algebraic,
                               unsigned precision)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open JSON file: " + filename);
    }

    // get the json representation
    json j;
    file >> j;
    std::vector<GateDef> gates;
    gates.reserve(j["gates"].size());

    for (const auto& g : j["gates"]) {
        const auto& names = g["names"];

        // plain string matrix
        std::string stringMatrix = g["matrix"].dump();

        // algebraic matrix if needed
        ComplexMatrix<ACN> matrix = 
        algebraic
            ? createAlgebraicMatrix(stringMatrix, precision)
            : createAlgebraicMatrix(stringMatrix, precision);  // TODO: some placeholder

        AtomicGateSemantics sem(std::move(matrix), std::move(stringMatrix));

        GateDef def{
            .name = names.at(0).get<std::string>(),
            .aliases = {},
            .kind = GateKind::Atomic,
            .semantics = std::variant<CompositeGateBody, AtomicGateSemantics>(
                std::in_place_type<AtomicGateSemantics>,
                std::move(sem))
        };

        // add the aliases
        for (size_t i = 1; i < names.size(); ++i) {
            def.aliases.push_back(names[i].get<std::string>());
        }
        
        // placeholder to have the same structure as composition gates
        def.argument_qubits.clear();
        for (unsigned i = 0; i <  g["num_qubits"].get<unsigned>(); ++i) {
            def.argument_qubits.emplace_back("q" + std::to_string(i));  
        }
        
        def.parameters = g.value("parameters", std::vector<std::string>{});

        gates.push_back(std::move(def));
    }

    return gates;
}

ComplexMatrix<ACN> createAlgebraicMatrix(const std::string& json_str,
                                         unsigned precision) {
    using ACN = AlgebraicComplexNumber<DenseNumberStore>;

    ACN zero(precision);
    MatrixBaker<ACN> baker{ &zero };

    json matrix_json = json::parse(json_str);

    std::size_t n = matrix_json.size();
    if (n == 0 || matrix_json[0].size() != n) {
        throw std::runtime_error("Matrix must be square");
    }

    std::vector<s64> flat_ints;
    flat_ints.reserve(n * n);

    for (const auto& row : matrix_json) {
        for (const auto& entry : row) {
            flat_ints.push_back(entry.get<int>());
        }
    }

    return baker.square_acn_matrix_from_ints(flat_ints);
}
