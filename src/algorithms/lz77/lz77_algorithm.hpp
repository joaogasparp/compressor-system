#ifndef COMPRESSOR_LZ77_ALGORITHM_HPP
#define COMPRESSOR_LZ77_ALGORITHM_HPP

#include "core/algorithm.hpp"

namespace compressor {

// LZ77 match structure
struct LZ77Match {
    uint16_t distance;   // Distance to the match
    uint8_t length;      // Length of the match
    uint8_t next_char;   // Character following the match
    
    LZ77Match() : distance(0), length(0), next_char(0) {}
    LZ77Match(uint16_t d, uint8_t l, uint8_t c) : distance(d), length(l), next_char(c) {}
    
    bool is_literal() const { return length == 0; }
};

class LZ77Algorithm : public Algorithm {
public:
    AlgorithmInfo get_info() const override;
    
    CompressionResult compress(const ByteVector& input, 
                             const CompressionConfig& config = CompressionConfig()) override;
    
    CompressionResult decompress(const ByteVector& input,
                               const CompressionConfig& config = CompressionConfig()) override;
    
    double estimate_ratio(const ByteVector& input) const override;

private:
    // LZ77 parameters
    static constexpr size_t WINDOW_SIZE = 4096;      // Look-back window size
    static constexpr size_t LOOKAHEAD_SIZE = 18;     // Look-ahead buffer size
    static constexpr size_t MIN_MATCH_LENGTH = 3;    // Minimum match length
    static constexpr size_t MAX_MATCH_LENGTH = 258;  // Maximum match length
    
    // Find the longest match in the sliding window
    LZ77Match find_longest_match(const ByteVector& input, size_t position);
    
    // Encode matches and literals
    ByteVector encode_matches(const std::vector<LZ77Match>& matches);
    std::vector<LZ77Match> decode_matches(const ByteVector& encoded);
    
    // Hash-based search for better performance
    class HashSearch {
    public:
        HashSearch();
        void update(const ByteVector& input, size_t position);
        LZ77Match find_match(const ByteVector& input, size_t position);
        void clear();
        
    private:
        static constexpr size_t HASH_BITS = 12;
        static constexpr size_t HASH_SIZE = 1 << HASH_BITS;
        static constexpr size_t HASH_MASK = HASH_SIZE - 1;
        
        std::vector<std::vector<size_t>> hash_table_;
        
        uint32_t hash3(uint8_t a, uint8_t b, uint8_t c) const;
    };
};

} // namespace compressor

#endif // COMPRESSOR_LZ77_ALGORITHM_HPP
