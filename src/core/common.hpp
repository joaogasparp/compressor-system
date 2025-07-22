#ifndef COMPRESSOR_COMMON_HPP
#define COMPRESSOR_COMMON_HPP

#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <cstdint>

namespace compressor {

// Type definitions
using ByteVector = std::vector<uint8_t>;
using TimePoint = std::chrono::high_resolution_clock::time_point;
using Duration = std::chrono::duration<double>;

// Forward declarations
class Algorithm;
class CompressionResult;
class BenchmarkResult;

// Compression statistics
struct CompressionStats {
    size_t original_size;
    size_t compressed_size;
    double compression_ratio;
    double compression_time_ms;
    double decompression_time_ms;
    uint32_t checksum;
    size_t threads_used;
    
    CompressionStats() 
        : original_size(0), compressed_size(0), compression_ratio(0.0)
        , compression_time_ms(0.0), decompression_time_ms(0.0)
        , checksum(0), threads_used(1) {}
};

// Algorithm metadata
struct AlgorithmInfo {
    std::string name;
    std::string description;
    bool supports_parallel;
    size_t min_block_size;
    
    AlgorithmInfo(const std::string& n, const std::string& desc, 
                  bool parallel = false, size_t min_size = 1024)
        : name(n), description(desc), supports_parallel(parallel), min_block_size(min_size) {}
};

// Configuration for compression
struct CompressionConfig {
    size_t block_size;
    size_t num_threads;
    bool verify_integrity;
    bool verbose;
    
    CompressionConfig() 
        : block_size(64 * 1024), num_threads(1), verify_integrity(true), verbose(false) {}
};

// Result of compression operation
class CompressionResult {
public:
    CompressionResult(bool success, const std::string& msg = "")
        : success_(success), message_(msg) {}
    
    bool is_success() const { return success_; }
    const std::string& message() const { return message_; }
    const CompressionStats& stats() const { return stats_; }
    CompressionStats& stats() { return stats_; }
    
    void set_data(ByteVector&& data) { data_ = std::move(data); }
    const ByteVector& data() const { return data_; }
    ByteVector& data() { return data_; }

private:
    bool success_;
    std::string message_;
    CompressionStats stats_;
    ByteVector data_;
};

// Exception classes
class CompressionException : public std::exception {
public:
    explicit CompressionException(const std::string& msg) : message_(msg) {}
    const char* what() const noexcept override { return message_.c_str(); }
private:
    std::string message_;
};

class DecompressionException : public std::exception {
public:
    explicit DecompressionException(const std::string& msg) : message_(msg) {}
    const char* what() const noexcept override { return message_.c_str(); }
private:
    std::string message_;
};

// Utility macros
#define COMPRESSOR_VERSION_MAJOR 1
#define COMPRESSOR_VERSION_MINOR 0
#define COMPRESSOR_VERSION_PATCH 0

} // namespace compressor

#endif // COMPRESSOR_COMMON_HPP
