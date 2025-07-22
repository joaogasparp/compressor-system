#include "algorithms/custom_hybrid/hybrid_algorithm.hpp"
#include "utils/crc.hpp"
#include <cmath>
#include <algorithm>
#include <unordered_map>

namespace compressor {

AlgorithmInfo HybridAlgorithm::get_info() const {
    return AlgorithmInfo(
        "hybrid",
        "Custom Hybrid Algorithm - Adaptive compression using RLE, Huffman, and LZ77 based on data characteristics",
        true,  // Supports parallel processing
        8192   // Optimal minimum block size
    );
}

CompressionResult HybridAlgorithm::compress(const ByteVector& input, const CompressionConfig& config) {
    if (input.empty()) {
        return CompressionResult(false, "Input data is empty");
    }
    
    initialize_algorithms();
    
    CompressionResult result(true);
    auto& stats = result.stats();
    
    stats.original_size = input.size();
    if (config.verify_integrity) {
        stats.checksum = utils::CRC32::calculate(input);
    }
    
    auto start_time = now();
    
    // Determine optimal block size
    size_t block_size = get_optimal_block_size(input.size());
    
    // Apply preprocessing to improve compression
    ByteVector preprocessed = apply_preprocessing(input);
    
    // Analyze input and classify blocks
    auto blocks = analyze_input(preprocessed, block_size);
    
    // Compress each block with optimal algorithm
    ByteVector compressed;
    compressed.reserve(input.size()); // Conservative estimate
    
    // Header: Hybrid signature and block count
    compressed.push_back('H');
    compressed.push_back('Y');
    compressed.push_back('B');
    compressed.push_back('R');
    
    uint32_t block_count = blocks.size();
    compressed.push_back((block_count >> 24) & 0xFF);
    compressed.push_back((block_count >> 16) & 0xFF);
    compressed.push_back((block_count >> 8) & 0xFF);
    compressed.push_back(block_count & 0xFF);
    
    // Compress blocks
    size_t total_compressed = 0;
    std::unordered_map<BlockType, size_t> algorithm_usage;
    
    for (const auto& block_info : blocks) {
        // Extract block data
        ByteVector block_data(preprocessed.begin() + block_info.start_offset,
                             preprocessed.begin() + block_info.start_offset + block_info.size);
        
        // Compress block
        ByteVector compressed_block = compress_block(block_data, block_info.type, config);
        
        // Store block header: type + original size + compressed size
        compressed.push_back(static_cast<uint8_t>(block_info.type));
        
        uint32_t original_size = block_info.size;
        compressed.push_back((original_size >> 24) & 0xFF);
        compressed.push_back((original_size >> 16) & 0xFF);
        compressed.push_back((original_size >> 8) & 0xFF);
        compressed.push_back(original_size & 0xFF);
        
        uint32_t compressed_size = compressed_block.size();
        compressed.push_back((compressed_size >> 24) & 0xFF);
        compressed.push_back((compressed_size >> 16) & 0xFF);
        compressed.push_back((compressed_size >> 8) & 0xFF);
        compressed.push_back(compressed_size & 0xFF);
        
        // Store compressed block data
        compressed.insert(compressed.end(), compressed_block.begin(), compressed_block.end());
        
        total_compressed += compressed_block.size();
        algorithm_usage[block_info.type]++;
    }
    
    // Apply postprocessing
    ByteVector final_compressed = apply_postprocessing(compressed);
    
    auto end_time = now();
    
    stats.compressed_size = final_compressed.size();
    stats.compression_ratio = static_cast<double>(stats.compressed_size) / stats.original_size;
    stats.compression_time_ms = duration_ms(start_time, end_time);
    stats.threads_used = config.num_threads;
    
    result.set_data(std::move(final_compressed));
    
    if (config.verbose) {
        printf("Hybrid compression: %.2f%% (%zu blocks)\n", 
               stats.compression_ratio * 100.0, blocks.size());
        printf("  RLE blocks: %zu, LZ77 blocks: %zu, Huffman blocks: %zu, Mixed blocks: %zu\n",
               algorithm_usage[BlockType::LOW_ENTROPY],
               algorithm_usage[BlockType::HIGH_REPETITION],
               algorithm_usage[BlockType::RANDOM],
               algorithm_usage[BlockType::MIXED]);
    }
    
    return result;
}

CompressionResult HybridAlgorithm::decompress(const ByteVector& input, const CompressionConfig& config) {
    if (input.empty()) {
        return CompressionResult(false, "Input data is empty");
    }
    
    initialize_algorithms();
    
    CompressionResult result(true);
    auto& stats = result.stats();
    
    auto start_time = now();
    
    try {
        // Check signature
        if (input.size() < 8 || input[0] != 'H' || input[1] != 'Y' || input[2] != 'B' || input[3] != 'R') {
            throw DecompressionException("Invalid hybrid compression signature");
        }
        
        // Read block count
        uint32_t block_count = (static_cast<uint32_t>(input[4]) << 24) |
                              (static_cast<uint32_t>(input[5]) << 16) |
                              (static_cast<uint32_t>(input[6]) << 8) |
                              static_cast<uint32_t>(input[7]);
        
        ByteVector decompressed;
        size_t offset = 8;
        
        // Decompress each block
        for (uint32_t i = 0; i < block_count; ++i) {
            if (offset + 9 > input.size()) {
                throw DecompressionException("Incomplete block header");
            }
            
            // Read block header
            BlockType type = static_cast<BlockType>(input[offset++]);
            
            uint32_t original_size = (static_cast<uint32_t>(input[offset]) << 24) |
                                   (static_cast<uint32_t>(input[offset + 1]) << 16) |
                                   (static_cast<uint32_t>(input[offset + 2]) << 8) |
                                   static_cast<uint32_t>(input[offset + 3]);
            offset += 4;
            
            uint32_t compressed_size = (static_cast<uint32_t>(input[offset]) << 24) |
                                     (static_cast<uint32_t>(input[offset + 1]) << 16) |
                                     (static_cast<uint32_t>(input[offset + 2]) << 8) |
                                     static_cast<uint32_t>(input[offset + 3]);
            offset += 4;
            
            if (offset + compressed_size > input.size()) {
                throw DecompressionException("Incomplete block data");
            }
            
            // Extract compressed block
            ByteVector compressed_block(input.begin() + offset, input.begin() + offset + compressed_size);
            offset += compressed_size;
            
            // Decompress block
            ByteVector decompressed_block = decompress_block(compressed_block, type, config);
            
            if (decompressed_block.size() != original_size) {
                throw DecompressionException("Block size mismatch after decompression");
            }
            
            decompressed.insert(decompressed.end(), decompressed_block.begin(), decompressed_block.end());
        }
        
        auto end_time = now();
        
        stats.original_size = decompressed.size();
        stats.compressed_size = input.size();
        stats.compression_ratio = static_cast<double>(stats.compressed_size) / stats.original_size;
        stats.decompression_time_ms = duration_ms(start_time, end_time);
        stats.threads_used = config.num_threads;
        
        if (config.verify_integrity) {
            stats.checksum = utils::CRC32::calculate(decompressed);
        }
        
        result.set_data(std::move(decompressed));
        
    } catch (const std::exception& e) {
        return CompressionResult(false, "Decompression failed: " + std::string(e.what()));
    }
    
    return result;
}

double HybridAlgorithm::estimate_ratio(const ByteVector& input) const {
    if (input.empty()) return 1.0;
    
    // Quick analysis for estimation
    double entropy = calculate_entropy(input);
    double repetition = calculate_repetition_score(input);
    
    // Estimate based on data characteristics
    if (entropy < LOW_ENTROPY_THRESHOLD) {
        return 0.2; // RLE works very well
    } else if (repetition > HIGH_REPETITION_THRESHOLD) {
        return 0.4; // LZ77 should be effective
    } else {
        return 0.6; // Huffman for random data
    }
}

size_t HybridAlgorithm::get_optimal_block_size(size_t input_size) const {
    // Adaptive block size based on input size
    if (input_size < 16384) {
        return std::max(MIN_BLOCK_SIZE, input_size / 4);
    } else if (input_size < 1048576) { // 1MB
        return 16384;
    } else {
        return std::min(MAX_BLOCK_SIZE, input_size / 64);
    }
}

void HybridAlgorithm::initialize_algorithms() {
    if (!rle_algo_) rle_algo_ = std::make_unique<RLEAlgorithm>();
    if (!huffman_algo_) huffman_algo_ = std::make_unique<HuffmanAlgorithm>();
    if (!lz77_algo_) lz77_algo_ = std::make_unique<LZ77Algorithm>();
}

std::vector<BlockInfo> HybridAlgorithm::analyze_input(const ByteVector& input, size_t block_size) {
    std::vector<BlockInfo> blocks;
    
    for (size_t offset = 0; offset < input.size(); offset += block_size) {
        size_t current_block_size = std::min(block_size, input.size() - offset);
        
        ByteVector block(input.begin() + offset, input.begin() + offset + current_block_size);
        
        double entropy = calculate_entropy(block);
        double repetition = calculate_repetition_score(block);
        
        BlockType type = classify_block(block);
        
        blocks.emplace_back(type, offset, current_block_size, entropy, repetition);
    }
    
    return blocks;
}

BlockType HybridAlgorithm::classify_block(const ByteVector& block) {
    double entropy = calculate_entropy(block);
    double repetition = calculate_repetition_score(block);
    double local_entropy = calculate_local_entropy(block, 256);
    
    // Multi-criteria classification
    if (entropy < LOW_ENTROPY_THRESHOLD) {
        return BlockType::LOW_ENTROPY;
    } else if (repetition > HIGH_REPETITION_THRESHOLD) {
        return BlockType::HIGH_REPETITION;
    } else if (local_entropy > 0.8 && entropy > 0.7) {
        return BlockType::RANDOM;
    } else {
        return BlockType::MIXED;
    }
}

double HybridAlgorithm::calculate_entropy(const ByteVector& data) const {
    if (data.empty()) return 0.0;
    
    std::unordered_map<uint8_t, size_t> frequencies;
    for (uint8_t byte : data) {
        frequencies[byte]++;
    }
    
    double entropy = 0.0;
    double size = static_cast<double>(data.size());
    
    for (const auto& pair : frequencies) {
        double probability = static_cast<double>(pair.second) / size;
        entropy -= probability * std::log2(probability);
    }
    
    return entropy / 8.0; // Normalize to [0,1]
}

double HybridAlgorithm::calculate_repetition_score(const ByteVector& data) const {
    if (data.size() < 4) return 0.0;
    
    size_t matches = 0;
    size_t total_comparisons = 0;
    
    // Look for repeating patterns
    for (size_t i = 0; i < data.size() - 3; ++i) {
        for (size_t j = i + 1; j < std::min(i + 64, data.size() - 3); ++j) {
            total_comparisons++;
            if (data[i] == data[j] && data[i+1] == data[j+1] && data[i+2] == data[j+2]) {
                matches++;
            }
        }
    }
    
    return total_comparisons > 0 ? static_cast<double>(matches) / total_comparisons : 0.0;
}

double HybridAlgorithm::calculate_local_entropy(const ByteVector& data, size_t window_size) const {
    if (data.size() < window_size) return calculate_entropy(data);
    
    double total_entropy = 0.0;
    size_t windows = 0;
    
    for (size_t i = 0; i <= data.size() - window_size; i += window_size / 2) {
        ByteVector window(data.begin() + i, data.begin() + i + window_size);
        total_entropy += calculate_entropy(window);
        windows++;
    }
    
    return windows > 0 ? total_entropy / windows : 0.0;
}

ByteVector HybridAlgorithm::compress_block(const ByteVector& block, BlockType type, const CompressionConfig& config) {
    CompressionResult result(false);
    
    switch (type) {
        case BlockType::LOW_ENTROPY:
            result = rle_algo_->compress(block, config);
            break;
        case BlockType::HIGH_REPETITION:
            result = lz77_algo_->compress(block, config);
            break;
        case BlockType::RANDOM:
            result = huffman_algo_->compress(block, config);
            break;
        case BlockType::MIXED: {
            // Try all algorithms and pick the best
            auto rle_result = rle_algo_->compress(block, config);
            auto lz77_result = lz77_algo_->compress(block, config);
            auto huffman_result = huffman_algo_->compress(block, config);
            
            // Select best compression ratio
            if (rle_result.is_success() && 
                (rle_result.stats().compressed_size <= lz77_result.stats().compressed_size) &&
                (rle_result.stats().compressed_size <= huffman_result.stats().compressed_size)) {
                result = std::move(rle_result);
            } else if (lz77_result.is_success() && 
                       (lz77_result.stats().compressed_size <= huffman_result.stats().compressed_size)) {
                result = std::move(lz77_result);
            } else {
                result = std::move(huffman_result);
            }
            break;
        }
    }
    
    if (!result.is_success()) {
        // Fallback to storing uncompressed
        return block;
    }
    
    return result.data();
}

ByteVector HybridAlgorithm::decompress_block(const ByteVector& block, BlockType type, const CompressionConfig& config) {
    CompressionResult result(false);
    
    switch (type) {
        case BlockType::LOW_ENTROPY:
            result = rle_algo_->decompress(block, config);
            break;
        case BlockType::HIGH_REPETITION:
            result = lz77_algo_->decompress(block, config);
            break;
        case BlockType::RANDOM:
        case BlockType::MIXED:
            result = huffman_algo_->decompress(block, config);
            break;
    }
    
    if (!result.is_success()) {
        throw DecompressionException("Failed to decompress block: " + result.message());
    }
    
    return result.data();
}

ByteVector HybridAlgorithm::apply_preprocessing(const ByteVector& input) {
    // Simple preprocessing: byte differencing for improved compression
    if (input.size() < 2) return input;
    
    ByteVector preprocessed;
    preprocessed.reserve(input.size());
    
    preprocessed.push_back(input[0]); // First byte unchanged
    
    for (size_t i = 1; i < input.size(); ++i) {
        // Store difference from previous byte
        int16_t diff = input[i] - input[i-1];
        preprocessed.push_back(static_cast<uint8_t>(diff & 0xFF));
    }
    
    return preprocessed;
}

ByteVector HybridAlgorithm::apply_postprocessing(const ByteVector& compressed) {
    // Simple postprocessing: could add additional entropy coding here
    return compressed;
}

} // namespace compressor
