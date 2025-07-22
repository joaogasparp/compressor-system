#ifndef COMPRESSOR_CLI_HPP
#define COMPRESSOR_CLI_HPP

#include "core/common.hpp"
#include "benchmark/benchmark.hpp"
#include <string>
#include <vector>

namespace compressor {
namespace cli {

// Command line argument structure
struct CliArgs {
    std::string command;
    std::string input_file;
    std::string output_file;
    std::string algorithm;
    std::vector<std::string> algorithms;
    size_t num_threads;
    size_t block_size;
    bool verbose;
    bool verify;
    bool interactive;
    bool help;
    
    // Benchmark specific
    std::string export_format;
    std::string export_file;
    size_t repetitions;
    
    CliArgs() : num_threads(1), block_size(0), verbose(false), 
                verify(true), interactive(false), help(false), repetitions(1) {}
};

// Command line parser
class ArgumentParser {
public:
    static CliArgs parse_arguments(int argc, char* argv[]);
    static void print_usage(const std::string& program_name);
    static void print_version();
    
private:
    static bool is_flag(const std::string& arg);
    static std::string get_flag_value(const std::string& arg);
};

// Interactive CLI mode
class InteractiveCli {
public:
    InteractiveCli();
    ~InteractiveCli();
    
    void run();
    
private:
    std::string current_file_;
    ByteVector current_data_;
    
    // Menu functions
    void show_main_menu();
    void handle_load_file();
    void handle_compress();
    void handle_decompress();
    void handle_benchmark();
    void handle_view_file_info();
    void handle_settings();
    void handle_help();
    
    // Utility functions
    bool load_file(const std::string& filename);
    void save_file(const std::string& filename, const ByteVector& data);
    void display_compression_result(const CompressionResult& result, const std::string& algorithm);
    void display_benchmark_result(const benchmark::BenchmarkResult& result);
    
    // Input helpers
    std::string get_input(const std::string& prompt);
    int get_choice(const std::string& prompt, int min_value, int max_value);
    bool confirm(const std::string& prompt);
    
    // Progress callback for benchmarks
    void progress_callback(const std::string& algorithm, int progress);
};

// Main CLI application
class CliApplication {
public:
    static int run(int argc, char* argv[]);
    
private:
    static int run_compress(const CliArgs& args);
    static int run_decompress(const CliArgs& args);
    static int run_benchmark(const CliArgs& args);
    static int run_interactive();
    
    static CompressionConfig create_compression_config(const CliArgs& args);
    static benchmark::BenchmarkConfig create_benchmark_config(const CliArgs& args);
    
    static void print_compression_result(const CompressionResult& result, const std::string& algorithm, bool verbose);
    static void save_benchmark_result(const benchmark::BenchmarkResult& result, 
                                    const std::string& format, const std::string& filename);
};

} // namespace cli
} // namespace compressor

#endif // COMPRESSOR_CLI_HPP
