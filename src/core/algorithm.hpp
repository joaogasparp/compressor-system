#ifndef COMPRESSOR_ALGORITHM_HPP
#define COMPRESSOR_ALGORITHM_HPP

#include "core/common.hpp"
#include <string>
#include <memory>

namespace compressor {

// Abstract base class for all compression algorithms
class Algorithm {
public:
    virtual ~Algorithm() = default;
    
    // Algorithm information
    virtual AlgorithmInfo get_info() const = 0;
    
    // Main compression/decompression interface
    virtual CompressionResult compress(const ByteVector& input, 
                                     const CompressionConfig& config = CompressionConfig()) = 0;
    
    virtual CompressionResult decompress(const ByteVector& input,
                                       const CompressionConfig& config = CompressionConfig()) = 0;
    
    // Optional: Estimate compression ratio without actual compression
    virtual double estimate_ratio(const ByteVector& input) const {
        (void)input; // Suppress unused parameter warning
        return 0.5; // Default conservative estimate
    }
    
    // Optional: Get optimal block size for this algorithm
    virtual size_t get_optimal_block_size(size_t input_size) const {
        (void)input_size;
        return 64 * 1024; // 64KB default
    }
    
protected:
    // Helper methods for derived classes
    static TimePoint now() {
        return std::chrono::high_resolution_clock::now();
    }
    
    static double duration_ms(TimePoint start, TimePoint end) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        return duration.count() / 1000.0;
    }
    
    // Template method for timing operations
    template<typename Func>
    static double time_operation(Func&& func) {
        auto start = now();
        func();
        auto end = now();
        return duration_ms(start, end);
    }
};

// Algorithm factory
class AlgorithmFactory {
public:
    static std::unique_ptr<Algorithm> create(const std::string& name);
    static std::vector<std::string> list_algorithms();
    static bool is_available(const std::string& name);
    
private:
    AlgorithmFactory() = default;
};

} // namespace compressor

#endif // COMPRESSOR_ALGORITHM_HPP
