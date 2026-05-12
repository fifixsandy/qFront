
// // Returns true if the loop's iteration values can be fully resolved at compile time.
// static bool isUnrollable(const LoopApplication& loop);

// // Returns the ordered list of string values the loop variable takes.
// // Only call if isUnrollable() returned true.
// static std::vector<std::string> getIterationValues(const LoopApplication& loop);

// // Deep-clones a single ProgramNodePtr (GateApplication, LoopApplication, ConditionalApplication).
// static ProgramNodePtr cloneNode(const ProgramNodePtr& node);

// // Deep-clones a Block (variables + body).
// static Block cloneBlock(const Block& block);

// // Replaces all whole-word occurrences of `var` with `value` in a string expression.
// // e.g. substituteVar("i + 2", "i", "3") -> "3 + 2"
// //      substituteVar("width", "i", "3") -> "width"  (no match)
// static std::string substituteVar(const std::string& expr,
//                                  const std::string& var,
//                                  const std::string& value);

// // Substitutes the loop variable throughout all RegisterRefs and expressions in a Block.
// static void substituteInBlock(Block& block,
//                                const std::string& var,
//                                const std::string& value);

// // Recursive worker — returns a new body with all unrollable loops expanded.
// // Recurses bottom-up: inner loops are processed before outer ones.
// static std::vector<ProgramNodePtr> unrollBlock(std::vector<ProgramNodePtr>& body);