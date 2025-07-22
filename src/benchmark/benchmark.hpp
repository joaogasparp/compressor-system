#ifndef COMPRESSOR_BENCHMARK_HPP
#define COMPRESSOR_BENCHMARK_HPP

#include "core/common.hpp"
#include "core/algorithm.hpp"
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace compressor {
namespace benchmark {

// Individual algorithm benchmark result
struct AlgorithmBenchmark {
    std::string algorithm_name;
    CompressionStats stats;
    bool success;
    std::string error_message;
    
    AlgorithmBenchmark(const std::string& name) 
        : algorithm_name(name), success(false) {}
};

// Complete benchmark result for all algorithms
class BenchmarkResult {
public:
    BenchmarkResult() = default;
    
    void add_result(AlgorithmBenchmark&& result);
    const std::vector<AlgorithmBenchmark>& get_results() const { return results_; }
    
    // Analysis methods
    AlgorithmBenchmark get_best_compression() const;
    AlgorithmBenchmark get_fastest_compression() const;
    AlgorithmBenchmark get_fastest_decompression() const;
    AlgorithmBenchmark get_best_overall() const;
    
    // Summary statistics
    double get_average_compression_ratio() const;
    double get_average_compression_time() const;
    size_t get_successful_count() const;
    
    // Export methods
    std::string to_text_report() const;
    std::string to_csv() const;
    std::string to_json() const;
    
private:
    std::vector<AlgorithmBenchmark> results_;
};

// Progress callback for long-running benchmarks
using ProgressCallback = std::function<void(const std::string& algorithm, int progress)>;

// Benchmark configuration
struct BenchmarkConfig {
    std::vector<std::string> algorithms;
    CompressionConfig compression_config;
    bool verify_roundtrip;
    bool measure_memory_usage;
    size_t repetitions;
    ProgressCallback progress_callback;
    
    BenchmarkConfig() 
        : verify_roundtrip(true), measure_memory_usage(false), repetitions(1) {}
};

// Main benchmark runner
class BenchmarkRunner {
public:
    BenchmarkRunner() = default;
    
    // Run benchmark on data
    BenchmarkResult run_benchmark(const ByteVector& data, const BenchmarkConfig& config);
    
    // Run benchmark on file
    BenchmarkResult run_file_benchmark(const std::string& filename, const BenchmarkConfig& config);
    
    // Run benchmark on multiple files
    std::vector<std::pair<std::string, BenchmarkResult>> run_multi_file_benchmark(
        const std::vector<std::string>& filenames, const BenchmarkConfig& config);
    
    // Static utility methods
    static BenchmarkConfig create_default_config();
    static BenchmarkConfig create_performance_config();
    static BenchmarkConfig create_comprehensive_config();
    
private:
    AlgorithmBenchmark benchmark_algorithm(const std::string& algorithm_name,
                                         const ByteVector& data,
                                         const BenchmarkConfig& config);
    
    bool verify_roundtrip(const ByteVector& original, const ByteVector& decompressed);
    void measure_memory_usage(const std::function<void()>& operation, CompressionStats& stats);
};

// Visualization helpers
class BenchmarkVisualizer {
public:
    // Generate ASCII bar chart
    static std::string create_compression_chart(const BenchmarkResult& result);
    static std::string create_speed_chart(const BenchmarkResult& result);
    static std::string create_combined_chart(const BenchmarkResult& result);
    
    // Terminal formatting
    static std::string format_size(size_t bytes);
    static std::string format_time(double milliseconds);
    static std::string format_ratio(double ratio);
    
private:
    static std::string create_bar(double value, double max_value, size_t width = 20);
    static std::string colorize(const std::string& text, const std::string& color);
};

} // namespace benchmark
} // namespace compressor

#endif // COMPRESSOR_BENCHMARK_HPP
