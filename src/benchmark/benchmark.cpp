#include "benchmark/benchmark.hpp"
#include "utils/file_utils.hpp"
#include "utils/crc.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <functional>
#include <iostream>

namespace compressor {
namespace benchmark {

void BenchmarkResult::add_result(AlgorithmBenchmark&& result) {
    results_.push_back(std::move(result));
}

AlgorithmBenchmark BenchmarkResult::get_best_compression() const {
    auto best = std::min_element(results_.begin(), results_.end(),
        [](const AlgorithmBenchmark& a, const AlgorithmBenchmark& b) {
            if (!a.success) return false;
            if (!b.success) return true;
            return a.stats.compression_ratio < b.stats.compression_ratio;
        });
    
    return best != results_.end() ? *best : AlgorithmBenchmark("none");
}

AlgorithmBenchmark BenchmarkResult::get_fastest_compression() const {
    auto fastest = std::min_element(results_.begin(), results_.end(),
        [](const AlgorithmBenchmark& a, const AlgorithmBenchmark& b) {
            if (!a.success) return false;
            if (!b.success) return true;
            return a.stats.compression_time_ms < b.stats.compression_time_ms;
        });
    
    return fastest != results_.end() ? *fastest : AlgorithmBenchmark("none");
}

AlgorithmBenchmark BenchmarkResult::get_fastest_decompression() const {
    auto fastest = std::min_element(results_.begin(), results_.end(),
        [](const AlgorithmBenchmark& a, const AlgorithmBenchmark& b) {
            if (!a.success) return false;
            if (!b.success) return true;
            return a.stats.decompression_time_ms < b.stats.decompression_time_ms;
        });
    
    return fastest != results_.end() ? *fastest : AlgorithmBenchmark("none");
}

AlgorithmBenchmark BenchmarkResult::get_best_overall() const {
    auto best = std::min_element(results_.begin(), results_.end(),
        [](const AlgorithmBenchmark& a, const AlgorithmBenchmark& b) {
            if (!a.success) return false;
            if (!b.success) return true;
            
            // Combined score: compression ratio (60%) + speed (40%)
            double score_a = a.stats.compression_ratio * 0.6 + 
                           (a.stats.compression_time_ms / 1000.0) * 0.4;
            double score_b = b.stats.compression_ratio * 0.6 + 
                           (b.stats.compression_time_ms / 1000.0) * 0.4;
            return score_a < score_b;
        });
    
    return best != results_.end() ? *best : AlgorithmBenchmark("none");
}

double BenchmarkResult::get_average_compression_ratio() const {
    if (results_.empty()) return 0.0;
    
    double sum = 0.0;
    size_t count = 0;
    
    for (const auto& result : results_) {
        if (result.success) {
            sum += result.stats.compression_ratio;
            count++;
        }
    }
    
    return count > 0 ? sum / count : 0.0;
}

double BenchmarkResult::get_average_compression_time() const {
    if (results_.empty()) return 0.0;
    
    double sum = 0.0;
    size_t count = 0;
    
    for (const auto& result : results_) {
        if (result.success) {
            sum += result.stats.compression_time_ms;
            count++;
        }
    }
    
    return count > 0 ? sum / count : 0.0;
}

size_t BenchmarkResult::get_successful_count() const {
    return std::count_if(results_.begin(), results_.end(),
        [](const AlgorithmBenchmark& result) { return result.success; });
}

std::string BenchmarkResult::to_text_report() const {
    std::ostringstream oss;
    
    oss << "=== Compression Benchmark Report ===\n\n";
    
    if (results_.empty()) {
        oss << "No results available.\n";
        return oss.str();
    }
    
    // Summary statistics
    oss << "Summary:\n";
    oss << "  Total algorithms tested: " << results_.size() << "\n";
    oss << "  Successful compressions: " << get_successful_count() << "\n";
    oss << "  Average compression ratio: " << std::fixed << std::setprecision(1) 
        << get_average_compression_ratio() * 100 << "%\n";
    oss << "  Average compression time: " << std::fixed << std::setprecision(2) 
        << get_average_compression_time() << " ms\n\n";
    
    // Best performers
    auto best_compression = get_best_compression();
    auto fastest_compression = get_fastest_compression();
    auto fastest_decompression = get_fastest_decompression();
    auto best_overall = get_best_overall();
    
    oss << "Best Performers:\n";
    if (best_compression.success) {
        oss << "  Best compression: " << best_compression.algorithm_name 
            << " (" << std::fixed << std::setprecision(1) 
            << best_compression.stats.compression_ratio * 100 << "%)\n";
    }
    if (fastest_compression.success) {
        oss << "  Fastest compression: " << fastest_compression.algorithm_name 
            << " (" << std::fixed << std::setprecision(2) 
            << fastest_compression.stats.compression_time_ms << " ms)\n";
    }
    if (fastest_decompression.success) {
        oss << "  Fastest decompression: " << fastest_decompression.algorithm_name 
            << " (" << std::fixed << std::setprecision(2) 
            << fastest_decompression.stats.decompression_time_ms << " ms)\n";
    }
    if (best_overall.success) {
        oss << "  Best overall: " << best_overall.algorithm_name << "\n";
    }
    oss << "\n";
    
    // Detailed results
    oss << "Detailed Results:\n";
    oss << std::setw(12) << "Algorithm" 
        << std::setw(12) << "Ratio" 
        << std::setw(12) << "Comp.Time" 
        << std::setw(12) << "Decomp.Time" 
        << std::setw(12) << "Threads"
        << std::setw(10) << "Status" << "\n";
    oss << std::string(70, '-') << "\n";
    
    for (const auto& result : results_) {
        oss << std::setw(12) << result.algorithm_name;
        
        if (result.success) {
            oss << std::setw(10) << std::fixed << std::setprecision(1) 
                << result.stats.compression_ratio * 100 << "%"
                << std::setw(10) << std::fixed << std::setprecision(1) 
                << result.stats.compression_time_ms << "ms"
                << std::setw(11) << std::fixed << std::setprecision(1) 
                << result.stats.decompression_time_ms << "ms"
                << std::setw(9) << result.stats.threads_used
                << std::setw(10) << "OK";
        } else {
            oss << std::setw(48) << "FAILED: " + result.error_message;
        }
        oss << "\n";
    }
    
    return oss.str();
}

std::string BenchmarkResult::to_csv() const {
    std::ostringstream oss;
    
    // CSV header
    oss << "Algorithm,Status,Original_Size,Compressed_Size,Compression_Ratio,"
        << "Compression_Time_ms,Decompression_Time_ms,Threads,Checksum,Error\n";
    
    for (const auto& result : results_) {
        oss << result.algorithm_name << ",";
        
        if (result.success) {
            oss << "SUCCESS," 
                << result.stats.original_size << ","
                << result.stats.compressed_size << ","
                << std::fixed << std::setprecision(6) << result.stats.compression_ratio << ","
                << std::fixed << std::setprecision(3) << result.stats.compression_time_ms << ","
                << std::fixed << std::setprecision(3) << result.stats.decompression_time_ms << ","
                << result.stats.threads_used << ","
                << "0x" << std::hex << result.stats.checksum << std::dec << ",";
        } else {
            oss << "FAILED,,,,,,,," << result.error_message;
        }
        oss << "\n";
    }
    
    return oss.str();
}

std::string BenchmarkResult::to_json() const {
    std::ostringstream oss;
    
    oss << "{\n";
    oss << "  \"benchmark_results\": [\n";
    
    for (size_t i = 0; i < results_.size(); ++i) {
        const auto& result = results_[i];
        
        oss << "    {\n";
        oss << "      \"algorithm\": \"" << result.algorithm_name << "\",\n";
        oss << "      \"success\": " << (result.success ? "true" : "false") << ",\n";
        
        if (result.success) {
            oss << "      \"stats\": {\n";
            oss << "        \"original_size\": " << result.stats.original_size << ",\n";
            oss << "        \"compressed_size\": " << result.stats.compressed_size << ",\n";
            oss << "        \"compression_ratio\": " << std::fixed << std::setprecision(6) 
                << result.stats.compression_ratio << ",\n";
            oss << "        \"compression_time_ms\": " << std::fixed << std::setprecision(3) 
                << result.stats.compression_time_ms << ",\n";
            oss << "        \"decompression_time_ms\": " << std::fixed << std::setprecision(3) 
                << result.stats.decompression_time_ms << ",\n";
            oss << "        \"threads_used\": " << result.stats.threads_used << ",\n";
            oss << "        \"checksum\": \"0x" << std::hex << result.stats.checksum << std::dec << "\"\n";
            oss << "      }\n";
        } else {
            oss << "      \"error\": \"" << result.error_message << "\"\n";
        }
        
        oss << "    }";
        if (i + 1 < results_.size()) oss << ",";
        oss << "\n";
    }
    
    oss << "  ],\n";
    oss << "  \"summary\": {\n";
    oss << "    \"total_algorithms\": " << results_.size() << ",\n";
    oss << "    \"successful_count\": " << get_successful_count() << ",\n";
    oss << "    \"average_compression_ratio\": " << std::fixed << std::setprecision(6) 
        << get_average_compression_ratio() << ",\n";
    oss << "    \"average_compression_time_ms\": " << std::fixed << std::setprecision(3) 
        << get_average_compression_time() << "\n";
    oss << "  }\n";
    oss << "}\n";
    
    return oss.str();
}

BenchmarkResult BenchmarkRunner::run_benchmark(const ByteVector& data, const BenchmarkConfig& config) {
    BenchmarkResult result;
    
    if (data.empty()) {
        AlgorithmBenchmark failed("error");
        failed.error_message = "Input data is empty";
        result.add_result(std::move(failed));
        return result;
    }
    
    std::vector<std::string> algorithms = config.algorithms;
    if (algorithms.empty()) {
        algorithms = AlgorithmFactory::list_algorithms();
    }
    
    for (size_t i = 0; i < algorithms.size(); ++i) {
        const auto& algorithm_name = algorithms[i];
        
        if (config.compression_config.verbose) {
            std::cout << "Testing " << algorithm_name << "...\n";
        }
        
        AlgorithmBenchmark bench_result = benchmark_algorithm(algorithm_name, data, config);
        result.add_result(std::move(bench_result));
    }
    
    if (config.compression_config.verbose) {
        std::cout << "Benchmark completed.\n";
    }
    
    return result;
}

BenchmarkResult BenchmarkRunner::run_file_benchmark(const std::string& filename, const BenchmarkConfig& config) {
    try {
        ByteVector data = utils::FileUtils::read_file(filename);
        return run_benchmark(data, config);
    } catch (const std::exception& e) {
        BenchmarkResult result;
        AlgorithmBenchmark failed("file_error");
        failed.error_message = "Failed to read file: " + std::string(e.what());
        result.add_result(std::move(failed));
        return result;
    }
}

std::vector<std::pair<std::string, BenchmarkResult>> BenchmarkRunner::run_multi_file_benchmark(
    const std::vector<std::string>& filenames, const BenchmarkConfig& config) {
    
    std::vector<std::pair<std::string, BenchmarkResult>> results;
    results.reserve(filenames.size());
    
    for (const auto& filename : filenames) {
        auto result = run_file_benchmark(filename, config);
        results.emplace_back(filename, std::move(result));
    }
    
    return results;
}

BenchmarkConfig BenchmarkRunner::create_default_config() {
    BenchmarkConfig config;
    config.algorithms = {"rle", "huffman", "lz77", "hybrid"};
    config.verify_roundtrip = true;
    config.repetitions = 1;
    return config;
}

BenchmarkConfig BenchmarkRunner::create_performance_config() {
    BenchmarkConfig config;
    config.algorithms = {"rle", "huffman", "lz77", "hybrid"};
    config.verify_roundtrip = false;
    config.repetitions = 3;
    config.compression_config.num_threads = 4;
    return config;
}

BenchmarkConfig BenchmarkRunner::create_comprehensive_config() {
    BenchmarkConfig config;
    config.algorithms = {"rle", "huffman", "lz77", "hybrid"};
    config.verify_roundtrip = true;
    config.measure_memory_usage = true;
    config.repetitions = 5;
    config.compression_config.verbose = true;
    return config;
}

AlgorithmBenchmark BenchmarkRunner::benchmark_algorithm(const std::string& algorithm_name,
                                                       const ByteVector& data,
                                                       const BenchmarkConfig& config) {
    AlgorithmBenchmark result(algorithm_name);
    
    try {
        auto algorithm = AlgorithmFactory::create(algorithm_name);
        if (!algorithm) {
            result.error_message = "Algorithm not available";
            return result;
        }
        
        CompressionStats best_stats;
        bool first_run = true;
        
        // Run multiple repetitions and keep the best result
        for (size_t rep = 0; rep < config.repetitions; ++rep) {
            // Compression
            auto compress_result = algorithm->compress(data, config.compression_config);
            if (!compress_result.is_success()) {
                result.error_message = compress_result.message();
                return result;
            }
            
            // Decompression
            auto decompress_result = algorithm->decompress(compress_result.data(), config.compression_config);
            if (!decompress_result.is_success()) {
                result.error_message = "Decompression failed: " + decompress_result.message();
                return result;
            }
            
            // Verify roundtrip if requested
            if (config.verify_roundtrip) {
                if (!verify_roundtrip(data, decompress_result.data())) {
                    result.error_message = "Roundtrip verification failed";
                    return result;
                }
            }
            
            // Update stats with best performance
            auto current_stats = compress_result.stats();
            current_stats.decompression_time_ms = decompress_result.stats().decompression_time_ms;
            
            if (first_run || current_stats.compression_time_ms < best_stats.compression_time_ms) {
                best_stats = current_stats;
                first_run = false;
            }
        }
        
        result.stats = best_stats;
        result.success = true;
        
    } catch (const std::exception& e) {
        result.error_message = e.what();
    }
    
    return result;
}

bool BenchmarkRunner::verify_roundtrip(const ByteVector& original, const ByteVector& decompressed) {
    if (original.size() != decompressed.size()) {
        return false;
    }
    
    return std::memcmp(original.data(), decompressed.data(), original.size()) == 0;
}

void BenchmarkRunner::measure_memory_usage(const std::function<void()>& operation, CompressionStats& stats) {
    // Simple implementation - could be enhanced with platform-specific memory measurement
    (void)operation;
    (void)stats;
    // TODO: Implement memory usage measurement
}

// BenchmarkVisualizer implementation
std::string BenchmarkVisualizer::create_compression_chart(const BenchmarkResult& result) {
    std::ostringstream oss;
    
    oss << "Compression Ratio Comparison:\n";
    oss << std::string(50, '=') << "\n";
    
    const auto& results = result.get_results();
    if (results.empty()) {
        oss << "No results to display.\n";
        return oss.str();
    }
    
    // Find max ratio for scaling
    double max_ratio = 0.0;
    for (const auto& res : results) {
        if (res.success) {
            max_ratio = std::max(max_ratio, res.stats.compression_ratio);
        }
    }
    
    for (const auto& res : results) {
        oss << std::setw(10) << res.algorithm_name << " ";
        
        if (res.success) {
            std::string bar = create_bar(res.stats.compression_ratio, max_ratio, 30);
            oss << bar << " " << std::fixed << std::setprecision(1) 
                << res.stats.compression_ratio * 100 << "%";
        } else {
            oss << "[" << std::setw(30) << "FAILED" << "] " << res.error_message;
        }
        oss << "\n";
    }
    
    return oss.str();
}

std::string BenchmarkVisualizer::create_speed_chart(const BenchmarkResult& result) {
    std::ostringstream oss;
    
    oss << "Compression Speed Comparison:\n";
    oss << std::string(50, '=') << "\n";
    
    const auto& results = result.get_results();
    if (results.empty()) {
        oss << "No results to display.\n";
        return oss.str();
    }
    
    // Find max time for scaling (invert for bar length)
    double max_time = 0.0;
    for (const auto& res : results) {
        if (res.success) {
            max_time = std::max(max_time, res.stats.compression_time_ms);
        }
    }
    
    for (const auto& res : results) {
        oss << std::setw(10) << res.algorithm_name << " ";
        
        if (res.success) {
            // Invert time for bar (shorter time = longer bar)
            double inverted_time = max_time - res.stats.compression_time_ms + (max_time * 0.1);
            std::string bar = create_bar(inverted_time, max_time * 1.1, 30);
            oss << bar << " " << format_time(res.stats.compression_time_ms);
        } else {
            oss << "[" << std::setw(30) << "FAILED" << "] " << res.error_message;
        }
        oss << "\n";
    }
    
    return oss.str();
}

std::string BenchmarkVisualizer::create_combined_chart(const BenchmarkResult& result) {
    std::ostringstream oss;
    
    oss << "Combined Performance Overview:\n";
    oss << std::string(60, '=') << "\n";
    oss << std::setw(12) << "Algorithm" 
        << std::setw(15) << "Compression" 
        << std::setw(15) << "Speed" 
        << std::setw(18) << "Overall Score" << "\n";
    oss << std::string(60, '-') << "\n";
    
    const auto& results = result.get_results();
    
    for (const auto& res : results) {
        oss << std::setw(12) << res.algorithm_name;
        
        if (res.success) {
            oss << std::setw(12) << format_ratio(res.stats.compression_ratio)
                << std::setw(12) << format_time(res.stats.compression_time_ms);
            
            // Calculate combined score
            double score = (1.0 - res.stats.compression_ratio) * 0.7 + 
                          (1000.0 / (res.stats.compression_time_ms + 1.0)) * 0.3;
            oss << std::setw(15) << std::fixed << std::setprecision(2) << score;
        } else {
            oss << std::setw(42) << "FAILED";
        }
        oss << "\n";
    }
    
    return oss.str();
}

std::string BenchmarkVisualizer::format_size(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    double size = static_cast<double>(bytes);
    int unit = 0;
    
    while (size >= 1024.0 && unit < 3) {
        size /= 1024.0;
        unit++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << size << " " << units[unit];
    return oss.str();
}

std::string BenchmarkVisualizer::format_time(double milliseconds) {
    std::ostringstream oss;
    if (milliseconds < 1000.0) {
        oss << std::fixed << std::setprecision(1) << milliseconds << "ms";
    } else {
        oss << std::fixed << std::setprecision(2) << milliseconds / 1000.0 << "s";
    }
    return oss.str();
}

std::string BenchmarkVisualizer::format_ratio(double ratio) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << ratio * 100 << "%";
    return oss.str();
}

std::string BenchmarkVisualizer::create_bar(double value, double max_value, size_t width) {
    if (max_value <= 0.0) return std::string(width, ' ');
    
    size_t filled = static_cast<size_t>((value / max_value) * width);
    filled = std::min(filled, width);
    
    std::string bar = "[";
    bar += std::string(filled, '#');
    bar += std::string(width - filled, ' ');
    bar += "]";
    
    return bar;
}

std::string BenchmarkVisualizer::colorize(const std::string& text, const std::string& color) {
    // Simple ANSI color codes
    if (color == "red") return "\033[31m" + text + "\033[0m";
    if (color == "green") return "\033[32m" + text + "\033[0m";
    if (color == "yellow") return "\033[33m" + text + "\033[0m";
    if (color == "blue") return "\033[34m" + text + "\033[0m";
    return text;
}

} // namespace benchmark
} // namespace compressor
