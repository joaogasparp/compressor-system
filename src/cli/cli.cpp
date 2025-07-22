#include "cli/cli.hpp"
#include "utils/file_utils.hpp"
#include "benchmark/benchmark.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <thread>

namespace compressor {
namespace cli {

CliArgs ArgumentParser::parse_arguments(int argc, char* argv[]) {
    CliArgs args;
    
    if (argc < 2) {
        args.help = true;
        return args;
    }
    
    args.command = argv[1];
    
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            args.help = true;
        } else if (arg == "-v" || arg == "--verbose") {
            args.verbose = true;
        } else if (arg == "--no-verify") {
            args.verify = false;
        } else if (arg == "-i" || arg == "--interactive") {
            args.interactive = true;
        } else if (arg == "-f" || arg == "--file") {
            if (i + 1 < argc) {
                args.input_file = argv[++i];
            }
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                args.output_file = argv[++i];
            }
        } else if (arg == "-a" || arg == "--algorithm") {
            if (i + 1 < argc) {
                args.algorithm = argv[++i];
            }
        } else if (arg == "--algorithms") {
            if (i + 1 < argc) {
                std::string algos = argv[++i];
                std::istringstream iss(algos);
                std::string algo;
                while (std::getline(iss, algo, ',')) {
                    args.algorithms.push_back(algo);
                }
            }
        } else if (arg == "-t" || arg == "--threads") {
            if (i + 1 < argc) {
                args.num_threads = std::stoul(argv[++i]);
            }
        } else if (arg == "-b" || arg == "--block-size") {
            if (i + 1 < argc) {
                args.block_size = std::stoul(argv[++i]);
            }
        } else if (arg == "--export-format") {
            if (i + 1 < argc) {
                args.export_format = argv[++i];
            }
        } else if (arg == "--export-file") {
            if (i + 1 < argc) {
                args.export_file = argv[++i];
            }
        } else if (arg == "-r" || arg == "--repetitions") {
            if (i + 1 < argc) {
                args.repetitions = std::stoul(argv[++i]);
            }
        } else if (!arg.empty() && arg[0] != '-') {
            // Positional argument
            if (args.input_file.empty()) {
                args.input_file = arg;
            } else if (args.output_file.empty()) {
                args.output_file = arg;
            }
        }
    }
    
    return args;
}

void ArgumentParser::print_usage(const std::string& program_name) {
    std::cout << "Compressor System - Advanced Data Compression Framework\n\n";
    std::cout << "Usage: " << program_name << " <command> [options]\n\n";
    
    std::cout << "Commands:\n";
    std::cout << "  compress     Compress a file\n";
    std::cout << "  decompress   Decompress a file\n";
    std::cout << "  benchmark    Run compression benchmarks\n";
    std::cout << "  interactive  Start interactive mode\n";
    std::cout << "  help         Show this help message\n";
    std::cout << "  version      Show version information\n\n";
    
    std::cout << "Options:\n";
    std::cout << "  -f, --file <file>        Input file path\n";
    std::cout << "  -o, --output <file>      Output file path\n";
    std::cout << "  -a, --algorithm <algo>   Compression algorithm (rle, huffman, lz77, hybrid)\n";
    std::cout << "  --algorithms <list>      Comma-separated list of algorithms for benchmark\n";
    std::cout << "  -t, --threads <num>      Number of threads to use\n";
    std::cout << "  -b, --block-size <size>  Block size for processing\n";
    std::cout << "  -v, --verbose            Enable verbose output\n";
    std::cout << "  --no-verify              Skip integrity verification\n";
    std::cout << "  -r, --repetitions <num>  Number of benchmark repetitions\n";
    std::cout << "  --export-format <fmt>    Export format (text, csv, json)\n";
    std::cout << "  --export-file <file>     Export benchmark results to file\n";
    std::cout << "  -h, --help               Show help message\n\n";
    
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << " compress -f input.txt -a huffman -o compressed.bin\n";
    std::cout << "  " << program_name << " decompress -f compressed.bin -o restored.txt\n";
    std::cout << "  " << program_name << " benchmark -f testfile.txt --export-format csv\n";
    std::cout << "  " << program_name << " interactive\n\n";
    
    std::cout << "Available algorithms:\n";
    auto algorithms = AlgorithmFactory::list_algorithms();
    for (const auto& algo : algorithms) {
        auto algorithm = AlgorithmFactory::create(algo);
        if (algorithm) {
            auto info = algorithm->get_info();
            std::cout << "  " << std::setw(10) << info.name << " - " << info.description << "\n";
        }
    }
}

void ArgumentParser::print_version() {
    std::cout << "Compressor System v" << COMPRESSOR_VERSION_MAJOR << "." 
              << COMPRESSOR_VERSION_MINOR << "." << COMPRESSOR_VERSION_PATCH << "\n";
    std::cout << "Advanced Data Compression Framework\n";
    std::cout << "Built with C++17 support\n";
}

// Interactive CLI implementation
InteractiveCli::InteractiveCli() {
    std::cout << "=== Compressor System Interactive Mode ===\n";
    std::cout << "Advanced Data Compression Framework\n\n";
}

InteractiveCli::~InteractiveCli() {
    std::cout << "\nGoodbye!\n";
}

void InteractiveCli::run() {
    while (true) {
        show_main_menu();
        
        int choice = get_choice("Select option", 0, 8);
        
        switch (choice) {
            case 1: handle_load_file(); break;
            case 2: handle_compress(); break;
            case 3: handle_decompress(); break;
            case 4: handle_benchmark(); break;
            case 5: handle_view_file_info(); break;
            case 6: handle_settings(); break;
            case 7: handle_help(); break;
            case 8: 
                std::cout << "Exiting...\n";
                return;
            case 0:
            default:
                std::cout << "Invalid choice. Please try again.\n\n";
                break;
        }
    }
}

void InteractiveCli::show_main_menu() {
    std::cout << "\n=== Main Menu ===\n";
    std::cout << "1. Load File\n";
    std::cout << "2. Compress Data\n";
    std::cout << "3. Decompress Data\n";
    std::cout << "4. Run Benchmark\n";
    std::cout << "5. View File Info\n";
    std::cout << "6. Settings\n";
    std::cout << "7. Help\n";
    std::cout << "8. Exit\n";
    
    if (!current_file_.empty()) {
        std::cout << "\nCurrent file: " << current_file_ 
                  << " (" << benchmark::BenchmarkVisualizer::format_size(current_data_.size()) << ")\n";
    }
}

void InteractiveCli::handle_load_file() {
    std::string filename = get_input("Enter file path: ");
    
    if (filename.empty()) {
        std::cout << "No file specified.\n";
        return;
    }
    
    if (load_file(filename)) {
        std::cout << "File loaded successfully!\n";
        std::cout << "Size: " << benchmark::BenchmarkVisualizer::format_size(current_data_.size()) << "\n";
    } else {
        std::cout << "Failed to load file.\n";
    }
}

void InteractiveCli::handle_compress() {
    if (current_data_.empty()) {
        std::cout << "No data loaded. Please load a file first.\n";
        return;
    }
    
    std::cout << "Available algorithms:\n";
    auto algorithms = AlgorithmFactory::list_algorithms();
    for (size_t i = 0; i < algorithms.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << algorithms[i] << "\n";
    }
    
    int choice = get_choice("Select algorithm", 1, algorithms.size());
    std::string selected_algo = algorithms[choice - 1];
    
    auto algorithm = AlgorithmFactory::create(selected_algo);
    if (!algorithm) {
        std::cout << "Failed to create algorithm.\n";
        return;
    }
    
    CompressionConfig config;
    config.verbose = true;
    
    std::cout << "Compressing with " << selected_algo << "...\n";
    auto result = algorithm->compress(current_data_, config);
    
    display_compression_result(result, selected_algo);
    
    if (result.is_success() && confirm("Save compressed data to file?")) {
        std::string output_file = get_input("Output file name: ");
        if (!output_file.empty()) {
            save_file(output_file, result.data());
        }
    }
}

void InteractiveCli::handle_decompress() {
    if (current_data_.empty()) {
        std::cout << "No data loaded. Please load a compressed file first.\n";
        return;
    }
    
    std::cout << "Attempting automatic decompression...\n";
    
    // Try each algorithm
    auto algorithms = AlgorithmFactory::list_algorithms();
    bool success = false;
    
    for (const auto& algo_name : algorithms) {
        auto algorithm = AlgorithmFactory::create(algo_name);
        if (!algorithm) continue;
        
        auto result = algorithm->decompress(current_data_);
        if (result.is_success()) {
            std::cout << "Successfully decompressed with " << algo_name << "!\n";
            std::cout << "Original size: " << benchmark::BenchmarkVisualizer::format_size(result.data().size()) << "\n";
            
            if (confirm("Save decompressed data to file?")) {
                std::string output_file = get_input("Output file name: ");
                if (!output_file.empty()) {
                    save_file(output_file, result.data());
                }
            }
            
            success = true;
            break;
        }
    }
    
    if (!success) {
        std::cout << "Failed to decompress with any algorithm.\n";
    }
}

void InteractiveCli::handle_benchmark() {
    if (current_data_.empty()) {
        std::cout << "No data loaded. Please load a file first.\n";
        return;
    }
    
    std::cout << "Running benchmark on current data...\n";
    
    benchmark::BenchmarkRunner runner;
    auto config = runner.create_default_config();
    
    // Note: Progress callback would be added here in a future version
    
    auto result = runner.run_benchmark(current_data_, config);
    display_benchmark_result(result);
    
    if (confirm("Export results to file?")) {
        std::cout << "1. Text\n2. CSV\n3. JSON\n";
        int format_choice = get_choice("Select format", 1, 3);
        
        std::string format;
        std::string extension;
        switch (format_choice) {
            case 1: format = "text"; extension = ".txt"; break;
            case 2: format = "csv"; extension = ".csv"; break;
            case 3: format = "json"; extension = ".json"; break;
        }
        
        std::string filename = get_input("Output file name (without extension): ");
        if (!filename.empty()) {
            filename += extension;
            
            std::string content;
            if (format == "text") {
                content = result.to_text_report();
            } else if (format == "csv") {
                content = result.to_csv();
            } else {
                content = result.to_json();
            }
            
            ByteVector data(content.begin(), content.end());
            save_file(filename, data);
        }
    }
}

void InteractiveCli::handle_view_file_info() {
    if (current_data_.empty()) {
        std::cout << "No data loaded.\n";
        return;
    }
    
    std::cout << "=== File Information ===\n";
    std::cout << "File: " << current_file_ << "\n";
    std::cout << "Size: " << benchmark::BenchmarkVisualizer::format_size(current_data_.size()) << "\n";
    
    // Calculate basic statistics
    std::unordered_map<uint8_t, size_t> frequencies;
    for (uint8_t byte : current_data_) {
        frequencies[byte]++;
    }
    
    std::cout << "Unique bytes: " << frequencies.size() << " / 256\n";
    
    // Find most common byte
    auto max_freq = std::max_element(frequencies.begin(), frequencies.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
    
    if (max_freq != frequencies.end()) {
        double percentage = (static_cast<double>(max_freq->second) / current_data_.size()) * 100.0;
        std::cout << "Most common byte: 0x" << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(max_freq->first) << std::dec 
                  << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
    }
    
    // Estimate compression ratios
    std::cout << "\nEstimated compression ratios:\n";
    auto algorithms = AlgorithmFactory::list_algorithms();
    for (const auto& algo_name : algorithms) {
        auto algorithm = AlgorithmFactory::create(algo_name);
        if (algorithm) {
            double ratio = algorithm->estimate_ratio(current_data_);
            std::cout << "  " << std::setw(10) << algo_name << ": " 
                      << std::fixed << std::setprecision(1) << ratio * 100 << "%\n";
        }
    }
}

void InteractiveCli::handle_settings() {
    std::cout << "=== Settings ===\n";
    std::cout << "1. View available algorithms\n";
    std::cout << "2. View system information\n";
    std::cout << "3. Back to main menu\n";
    
    int choice = get_choice("Select option", 1, 3);
    
    switch (choice) {
        case 1: {
            std::cout << "\nAvailable algorithms:\n";
            auto algorithms = AlgorithmFactory::list_algorithms();
            for (const auto& algo_name : algorithms) {
                auto algorithm = AlgorithmFactory::create(algo_name);
                if (algorithm) {
                    auto info = algorithm->get_info();
                    std::cout << "  " << std::setw(12) << info.name << " - " << info.description << "\n";
                    std::cout << "    Parallel: " << (info.supports_parallel ? "Yes" : "No") 
                              << ", Min block: " << info.min_block_size << " bytes\n";
                }
            }
            break;
        }
        case 2: {
            std::cout << "\nSystem Information:\n";
            std::cout << "  Compressor Version: " << COMPRESSOR_VERSION_MAJOR << "." 
                      << COMPRESSOR_VERSION_MINOR << "." << COMPRESSOR_VERSION_PATCH << "\n";
            std::cout << "  Available threads: " << std::thread::hardware_concurrency() << "\n";
            break;
        }
        case 3:
            return;
    }
}

void InteractiveCli::handle_help() {
    std::cout << "\n=== Help ===\n";
    std::cout << "This is the interactive mode of the Compressor System.\n\n";
    std::cout << "Steps to use:\n";
    std::cout << "1. Load a file using 'Load File'\n";
    std::cout << "2. Use 'Compress Data' to compress with different algorithms\n";
    std::cout << "3. Use 'Decompress Data' to restore compressed files\n";
    std::cout << "4. Use 'Run Benchmark' to compare all algorithms\n";
    std::cout << "5. Use 'View File Info' to see file statistics\n\n";
    std::cout << "The system supports RLE, Huffman, LZ77, and custom hybrid compression.\n";
    std::cout << "All operations include integrity verification by default.\n\n";
}

bool InteractiveCli::load_file(const std::string& filename) {
    try {
        current_data_ = utils::FileUtils::read_file(filename);
        current_file_ = filename;
        return true;
    } catch (const std::exception& e) {
        std::cout << "Error loading file: " << e.what() << "\n";
        return false;
    }
}

void InteractiveCli::save_file(const std::string& filename, const ByteVector& data) {
    if (utils::FileUtils::write_file(filename, data)) {
        std::cout << "File saved successfully: " << filename << "\n";
    } else {
        std::cout << "Failed to save file: " << filename << "\n";
    }
}

void InteractiveCli::display_compression_result(const CompressionResult& result, const std::string& algorithm) {
    if (result.is_success()) {
        const auto& stats = result.stats();
        std::cout << "Compression successful!\n";
        std::cout << "  Algorithm: " << algorithm << "\n";
        std::cout << "  Original size: " << benchmark::BenchmarkVisualizer::format_size(stats.original_size) << "\n";
        std::cout << "  Compressed size: " << benchmark::BenchmarkVisualizer::format_size(stats.compressed_size) << "\n";
        std::cout << "  Compression ratio: " << std::fixed << std::setprecision(1) 
                  << stats.compression_ratio * 100 << "%\n";
        std::cout << "  Time: " << benchmark::BenchmarkVisualizer::format_time(stats.compression_time_ms) << "\n";
    } else {
        std::cout << "Compression failed: " << result.message() << "\n";
    }
}

void InteractiveCli::display_benchmark_result(const benchmark::BenchmarkResult& result) {
    std::cout << "\n" << benchmark::BenchmarkVisualizer::create_compression_chart(result) << "\n";
    std::cout << benchmark::BenchmarkVisualizer::create_speed_chart(result) << "\n";
    
    auto best = result.get_best_compression();
    if (best.success) {
        std::cout << "Best compression: " << best.algorithm_name 
                  << " (" << std::fixed << std::setprecision(1) 
                  << best.stats.compression_ratio * 100 << "%)\n";
    }
}

std::string InteractiveCli::get_input(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

int InteractiveCli::get_choice(const std::string& prompt, int min_value, int max_value) {
    while (true) {
        std::cout << prompt << " (" << min_value << "-" << max_value << "): ";
        std::string input;
        std::getline(std::cin, input);
        
        try {
            int choice = std::stoi(input);
            if (choice >= min_value && choice <= max_value) {
                return choice;
            }
        } catch (...) {
            // Invalid input
        }
        
        std::cout << "Invalid choice. Please enter a number between " 
                  << min_value << " and " << max_value << ".\n";
    }
}

bool InteractiveCli::confirm(const std::string& prompt) {
    std::cout << prompt << " (y/n): ";
    std::string input;
    std::getline(std::cin, input);
    return !input.empty() && (input[0] == 'y' || input[0] == 'Y');
}

void InteractiveCli::progress_callback(const std::string& algorithm, int progress) {
    if (progress == 100) {
        std::cout << "Completed: " << algorithm << "\n";
    } else {
        std::cout << "Testing " << algorithm << "... " << progress << "%\r" << std::flush;
    }
}

// Main CLI application
int CliApplication::run(int argc, char* argv[]) {
    try {
        auto args = ArgumentParser::parse_arguments(argc, argv);
        
        if (args.help || args.command == "help") {
            ArgumentParser::print_usage(argv[0]);
            return 0;
        }
        
        if (args.command == "version") {
            ArgumentParser::print_version();
            return 0;
        }
        
        if (args.command == "interactive" || args.interactive) {
            return run_interactive();
        }
        
        if (args.command == "compress") {
            return run_compress(args);
        }
        
        if (args.command == "decompress") {
            return run_decompress(args);
        }
        
        if (args.command == "benchmark") {
            return run_benchmark(args);
        }
        
        std::cerr << "Unknown command: " << args.command << "\n";
        std::cerr << "Use '" << argv[0] << " help' for usage information.\n";
        return 1;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

int CliApplication::run_compress(const CliArgs& args) {
    if (args.input_file.empty()) {
        std::cerr << "Input file not specified. Use -f or --file option.\n";
        return 1;
    }
    
    if (args.algorithm.empty()) {
        std::cerr << "Algorithm not specified. Use -a or --algorithm option.\n";
        return 1;
    }
    
    // Load input file
    ByteVector data;
    try {
        data = utils::FileUtils::read_file(args.input_file);
    } catch (const std::exception& e) {
        std::cerr << "Failed to read input file: " << e.what() << "\n";
        return 1;
    }
    
    // Create algorithm
    auto algorithm = AlgorithmFactory::create(args.algorithm);
    if (!algorithm) {
        std::cerr << "Unknown algorithm: " << args.algorithm << "\n";
        return 1;
    }
    
    // Compress
    auto config = create_compression_config(args);
    auto result = algorithm->compress(data, config);
    
    print_compression_result(result, args.algorithm, args.verbose);
    
    if (!result.is_success()) {
        return 1;
    }
    
    // Save output
    std::string output_file = args.output_file;
    if (output_file.empty()) {
        output_file = args.input_file + ".compressed";
    }
    
    if (!utils::FileUtils::write_file(output_file, result.data())) {
        std::cerr << "Failed to write output file: " << output_file << "\n";
        return 1;
    }
    
    std::cout << "Compressed file saved: " << output_file << "\n";
    return 0;
}

int CliApplication::run_decompress(const CliArgs& args) {
    if (args.input_file.empty()) {
        std::cerr << "Input file not specified. Use -f or --file option.\n";
        return 1;
    }
    
    // Load input file
    ByteVector data;
    try {
        data = utils::FileUtils::read_file(args.input_file);
    } catch (const std::exception& e) {
        std::cerr << "Failed to read input file: " << e.what() << "\n";
        return 1;
    }
    
    auto config = create_compression_config(args);
    
    // Try specified algorithm first, then auto-detect
    std::vector<std::string> try_algorithms;
    if (!args.algorithm.empty()) {
        try_algorithms.push_back(args.algorithm);
    }
    auto all_algorithms = AlgorithmFactory::list_algorithms();
    try_algorithms.insert(try_algorithms.end(), all_algorithms.begin(), all_algorithms.end());
    
    for (const auto& algo_name : try_algorithms) {
        auto algorithm = AlgorithmFactory::create(algo_name);
        if (!algorithm) continue;
        
        auto result = algorithm->decompress(data, config);
        if (result.is_success()) {
            std::cout << "Successfully decompressed with " << algo_name << "\n";
            
            // Save output
            std::string output_file = args.output_file;
            if (output_file.empty()) {
                output_file = args.input_file + ".decompressed";
            }
            
            if (!utils::FileUtils::write_file(output_file, result.data())) {
                std::cerr << "Failed to write output file: " << output_file << "\n";
                return 1;
            }
            
            std::cout << "Decompressed file saved: " << output_file << "\n";
            
            if (args.verbose) {
                const auto& stats = result.stats();
                std::cout << "Original size: " << benchmark::BenchmarkVisualizer::format_size(stats.original_size) << "\n";
                std::cout << "Decompression time: " << benchmark::BenchmarkVisualizer::format_time(stats.decompression_time_ms) << "\n";
            }
            
            return 0;
        }
    }
    
    std::cerr << "Failed to decompress file with any available algorithm.\n";
    return 1;
}

int CliApplication::run_benchmark(const CliArgs& args) {
    if (args.input_file.empty()) {
        std::cerr << "Input file not specified. Use -f or --file option.\n";
        return 1;
    }
    
    benchmark::BenchmarkRunner runner;
    auto config = create_benchmark_config(args);
    
    if (args.verbose) {
        std::cout << "Running verbose benchmark...\n";
    }
    
    auto result = runner.run_file_benchmark(args.input_file, config);
    
    // Display results
    std::cout << benchmark::BenchmarkVisualizer::create_compression_chart(result) << "\n";
    if (args.verbose) {
        std::cout << benchmark::BenchmarkVisualizer::create_speed_chart(result) << "\n";
        std::cout << result.to_text_report() << "\n";
    }
    
    // Export if requested
    if (!args.export_format.empty() || !args.export_file.empty()) {
        save_benchmark_result(result, args.export_format, args.export_file);
    }
    
    return 0;
}

int CliApplication::run_interactive() {
    InteractiveCli cli;
    cli.run();
    return 0;
}

CompressionConfig CliApplication::create_compression_config(const CliArgs& args) {
    CompressionConfig config;
    config.num_threads = args.num_threads;
    config.verbose = args.verbose;
    config.verify_integrity = args.verify;
    
    if (args.block_size > 0) {
        config.block_size = args.block_size;
    }
    
    return config;
}

benchmark::BenchmarkConfig CliApplication::create_benchmark_config(const CliArgs& args) {
    benchmark::BenchmarkConfig config;
    
    if (!args.algorithms.empty()) {
        config.algorithms = args.algorithms;
    }
    
    config.compression_config = create_compression_config(args);
    config.repetitions = args.repetitions;
    
    return config;
}

void CliApplication::print_compression_result(const CompressionResult& result, const std::string& algorithm, bool verbose) {
    if (result.is_success()) {
        const auto& stats = result.stats();
        std::cout << "Compression successful with " << algorithm << "\n";
        
        if (verbose) {
            std::cout << "  Original size: " << benchmark::BenchmarkVisualizer::format_size(stats.original_size) << "\n";
            std::cout << "  Compressed size: " << benchmark::BenchmarkVisualizer::format_size(stats.compressed_size) << "\n";
        }
        
        std::cout << "  Compression ratio: " << std::fixed << std::setprecision(1) 
                  << stats.compression_ratio * 100 << "%\n";
        std::cout << "  Time: " << benchmark::BenchmarkVisualizer::format_time(stats.compression_time_ms) << "\n";
        
        if (verbose && stats.threads_used > 1) {
            std::cout << "  Threads used: " << stats.threads_used << "\n";
        }
    } else {
        std::cerr << "Compression failed: " << result.message() << "\n";
    }
}

void CliApplication::save_benchmark_result(const benchmark::BenchmarkResult& result, 
                                         const std::string& format, const std::string& filename) {
    std::string content;
    std::string actual_filename = filename;
    
    if (format == "csv" || actual_filename.empty()) {
        content = result.to_csv();
        if (actual_filename.empty()) actual_filename = "benchmark_results.csv";
    } else if (format == "json") {
        content = result.to_json();
        if (actual_filename.empty()) actual_filename = "benchmark_results.json";
    } else {
        content = result.to_text_report();
        if (actual_filename.empty()) actual_filename = "benchmark_results.txt";
    }
    
    ByteVector data(content.begin(), content.end());
    if (utils::FileUtils::write_file(actual_filename, data)) {
        std::cout << "Benchmark results exported to: " << actual_filename << "\n";
    } else {
        std::cerr << "Failed to export results to: " << actual_filename << "\n";
    }
}

} // namespace cli
} // namespace compressor
