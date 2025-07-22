#ifndef COMPRESSOR_HYBRID_ALGORITHM_HPP
#define COMPRESSOR_HYBRID_ALGORITHM_HPP

#include "core/algorithm.hpp"
#include "algorithms/rle/rle_algorithm.hpp"
#include "algorithms/huffman/huffman_algorithm.hpp"
#include "algorithms/lz77/lz77_algorithm.hpp"
#include <memory>

namespace compressor {

// Block classification for adaptive compression
enum class BlockType {
    LOW_ENTROPY,     // Use RLE
    HIGH_REPETITION, // Use LZ77
    RANDOM,          // Use Huffman
    MIXED            // Use hybrid approach
};

// Block metadata
struct BlockInfo {
    BlockType type;
    size_t start_offset;
    size_t size;
    double entropy;
    double repetition_score;
    
    BlockInfo(BlockType t, size_t start, size_t sz, double ent, double rep)
        : type(t), start_offset(start), size(sz), entropy(ent), repetition_score(rep) {}
};

class HybridAlgorithm : public Algorithm {
public:
    AlgorithmInfo get_info() const override;
    
    CompressionResult compress(const ByteVector& input, 
                             const CompressionConfig& config = CompressionConfig()) override;
    
    CompressionResult decompress(const ByteVector& input,
                               const CompressionConfig& config = CompressionConfig()) override;
    
    double estimate_ratio(const ByteVector& input) const override;
    size_t get_optimal_block_size(size_t input_size) const override;

private:
    // Block size for analysis (adaptive based on input size)
    static constexpr size_t MIN_BLOCK_SIZE = 4096;
    static constexpr size_t MAX_BLOCK_SIZE = 65536;
    
    // Thresholds for algorithm selection
    static constexpr double LOW_ENTROPY_THRESHOLD = 0.3;
    static constexpr double HIGH_REPETITION_THRESHOLD = 0.6;
    static constexpr double MIN_IMPROVEMENT_RATIO = 0.95;
    
    // Algorithm instances
    std::unique_ptr<RLEAlgorithm> rle_algo_;
    std::unique_ptr<HuffmanAlgorithm> huffman_algo_;
    std::unique_ptr<LZ77Algorithm> lz77_algo_;
    
    // Analysis methods
    std::vector<BlockInfo> analyze_input(const ByteVector& input, size_t block_size);
    BlockType classify_block(const ByteVector& block);
    
    // Entropy and repetition analysis
    double calculate_entropy(const ByteVector& data) const;
    double calculate_repetition_score(const ByteVector& data) const;
    double calculate_local_entropy(const ByteVector& data, size_t window_size) const;
    
    // Compression strategy selection
    std::string select_best_algorithm(const ByteVector& block, const CompressionConfig& config);
    
    // Block processing
    ByteVector compress_block(const ByteVector& block, BlockType type, const CompressionConfig& config);
    ByteVector decompress_block(const ByteVector& block, BlockType type, const CompressionConfig& config);
    
    // Advanced hybrid techniques
    ByteVector apply_preprocessing(const ByteVector& input);
    ByteVector apply_postprocessing(const ByteVector& compressed);
    
    // Context-based prediction for better compression
    ByteVector apply_context_modeling(const ByteVector& input);
    
    // Initialize algorithm instances
    void initialize_algorithms();
};

} // namespace compressor

#endif // COMPRESSOR_HYBRID_ALGORITHM_HPP
