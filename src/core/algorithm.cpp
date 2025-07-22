#include "core/algorithm.hpp"
#include "algorithms/rle/rle_algorithm.hpp"
#include "algorithms/huffman/huffman_algorithm.hpp"
#include "algorithms/lz77/lz77_algorithm.hpp"
#include <unordered_map>
#include <functional>

namespace compressor {

using AlgorithmCreator = std::function<std::unique_ptr<Algorithm>()>;

static std::unordered_map<std::string, AlgorithmCreator> algorithm_registry = {
    {"rle", []() { return std::make_unique<RLEAlgorithm>(); }},
    {"huffman", []() { return std::make_unique<HuffmanAlgorithm>(); }},
    {"lz77", []() { return std::make_unique<LZ77Algorithm>(); }}
};

std::unique_ptr<Algorithm> AlgorithmFactory::create(const std::string& name) {
    auto it = algorithm_registry.find(name);
    if (it != algorithm_registry.end()) {
        return it->second();
    }
    return nullptr;
}

std::vector<std::string> AlgorithmFactory::list_algorithms() {
    std::vector<std::string> algorithms;
    algorithms.reserve(algorithm_registry.size());
    
    for (const auto& pair : algorithm_registry) {
        algorithms.push_back(pair.first);
    }
    
    return algorithms;
}

bool AlgorithmFactory::is_available(const std::string& name) {
    return algorithm_registry.find(name) != algorithm_registry.end();
}

} // namespace compressor
