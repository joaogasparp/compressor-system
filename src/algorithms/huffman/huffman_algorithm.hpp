#ifndef COMPRESSOR_HUFFMAN_ALGORITHM_HPP
#define COMPRESSOR_HUFFMAN_ALGORITHM_HPP

#include "core/algorithm.hpp"
#include <memory>
#include <unordered_map>
#include <queue>

namespace compressor {

// Huffman tree node
struct HuffmanNode {
    uint8_t byte;
    size_t frequency;
    std::unique_ptr<HuffmanNode> left;
    std::unique_ptr<HuffmanNode> right;
    
    HuffmanNode(uint8_t b, size_t freq) 
        : byte(b), frequency(freq), left(nullptr), right(nullptr) {}
    
    HuffmanNode(size_t freq, std::unique_ptr<HuffmanNode> l, std::unique_ptr<HuffmanNode> r)
        : byte(0), frequency(freq), left(std::move(l)), right(std::move(r)) {}
    
    bool is_leaf() const { return !left && !right; }
};

// Huffman code representation
struct HuffmanCode {
    uint32_t code;
    uint8_t length;
    
    HuffmanCode() : code(0), length(0) {}
    HuffmanCode(uint32_t c, uint8_t len) : code(c), length(len) {}
};

class HuffmanAlgorithm : public Algorithm {
public:
    AlgorithmInfo get_info() const override;
    
    CompressionResult compress(const ByteVector& input, 
                             const CompressionConfig& config = CompressionConfig()) override;
    
    CompressionResult decompress(const ByteVector& input,
                               const CompressionConfig& config = CompressionConfig()) override;
    
    double estimate_ratio(const ByteVector& input) const override;

private:
    // Build Huffman tree from frequency table
    std::unique_ptr<HuffmanNode> build_tree(const std::unordered_map<uint8_t, size_t>& frequencies);
    
    // Generate codes from tree
    std::unordered_map<uint8_t, HuffmanCode> generate_codes(const HuffmanNode* root);
    void generate_codes_recursive(const HuffmanNode* node, uint32_t code, uint8_t depth,
                                  std::unordered_map<uint8_t, HuffmanCode>& codes);
    
    // Serialize/deserialize tree for storage
    ByteVector serialize_tree(const HuffmanNode* root);
    std::unique_ptr<HuffmanNode> deserialize_tree(const ByteVector& data, size_t& offset);
    
    // Bit manipulation utilities
    class BitWriter {
    public:
        explicit BitWriter(ByteVector& output) : output_(output), current_byte_(0), bits_used_(0) {}
        ~BitWriter() { flush(); }
        
        void write_bits(uint32_t value, uint8_t count);
        void flush();
        
    private:
        ByteVector& output_;
        uint8_t current_byte_;
        uint8_t bits_used_;
    };
    
    class BitReader {
    public:
        explicit BitReader(const ByteVector& input) : input_(input), position_(0), current_byte_(0), bits_available_(0) {}
        
        uint32_t read_bits(uint8_t count);
        bool has_more() const;
        
    private:
        const ByteVector& input_;
        size_t position_;
        uint8_t current_byte_;
        uint8_t bits_available_;
    };
    
    // Calculate entropy for estimation
    double calculate_entropy(const std::unordered_map<uint8_t, size_t>& frequencies, size_t total_size) const;
};

} // namespace compressor

#endif // COMPRESSOR_HUFFMAN_ALGORITHM_HPP
