#ifndef COMPRESSOR_RLE_ALGORITHM_HPP
#define COMPRESSOR_RLE_ALGORITHM_HPP

#include "core/algorithm.hpp"

namespace compressor {

class RLEAlgorithm : public Algorithm {
public:
    AlgorithmInfo get_info() const override;
    
    CompressionResult compress(const ByteVector& input, 
                             const CompressionConfig& config = CompressionConfig()) override;
    
    CompressionResult decompress(const ByteVector& input,
                               const CompressionConfig& config = CompressionConfig()) override;
    
    double estimate_ratio(const ByteVector& input) const override;
    
private:
    // Simple RLE: encode runs of identical bytes
    ByteVector encode_rle(const ByteVector& input);
    ByteVector decode_rle(const ByteVector& input);
    
    // Enhanced RLE with better handling of non-repeating sequences
    ByteVector encode_enhanced_rle(const ByteVector& input);
    ByteVector decode_enhanced_rle(const ByteVector& input);
    
    // Calculate entropy to decide which encoding to use
    double calculate_entropy(const ByteVector& input) const;
};

} // namespace compressor

#endif // COMPRESSOR_RLE_ALGORITHM_HPP
