#include "algorithms/rle/rle_algorithm.hpp"
#include "utils/crc.hpp"
#include <cmath>
#include <unordered_map>

namespace compressor {

AlgorithmInfo RLEAlgorithm::get_info() const {
    return AlgorithmInfo(
        "rle",
        "Run Length Encoding - Efficient for data with many consecutive identical bytes",
        false, // No parallel support in basic implementation
        1024   // Minimum block size
    );
}

CompressionResult RLEAlgorithm::compress(const ByteVector& input, const CompressionConfig& config) {
    if (input.empty()) {
        return CompressionResult(false, "Input data is empty");
    }
    
    CompressionResult result(true);
    auto& stats = result.stats();
    
    // Record original size and calculate checksum
    stats.original_size = input.size();
    if (config.verify_integrity) {
        stats.checksum = utils::CRC32::calculate(input);
    }
    
    // Time the compression
    auto start_time = now();
    
    // Choose encoding method based on data characteristics
    double entropy = calculate_entropy(input);
    ByteVector compressed;
    
    if (entropy < 0.5) {
        // Low entropy - use enhanced RLE
        compressed = encode_enhanced_rle(input);
    } else {
        // High entropy - use simple RLE
        compressed = encode_rle(input);
    }
    
    auto end_time = now();
    
    // Update statistics
    stats.compressed_size = compressed.size();
    stats.compression_ratio = static_cast<double>(stats.compressed_size) / stats.original_size;
    stats.compression_time_ms = duration_ms(start_time, end_time);
    stats.threads_used = 1;
    
    result.set_data(std::move(compressed));
    
    if (config.verbose) {
        printf("RLE compression: %.2f%% (entropy: %.3f)\n", 
               stats.compression_ratio * 100.0, entropy);
    }
    
    return result;
}

CompressionResult RLEAlgorithm::decompress(const ByteVector& input, const CompressionConfig& config) {
    if (input.empty()) {
        return CompressionResult(false, "Input data is empty");
    }
    
    CompressionResult result(true);
    auto& stats = result.stats();
    
    auto start_time = now();
    
    ByteVector decompressed;
    
    try {
        // Try enhanced RLE first (it has a header to identify itself)
        if (input.size() > 1 && input[0] == 0xE1) {
            decompressed = decode_enhanced_rle(input);
        } else {
            decompressed = decode_rle(input);
        }
    } catch (const std::exception& e) {
        return CompressionResult(false, "Decompression failed: " + std::string(e.what()));
    }
    
    auto end_time = now();
    
    // Update statistics
    stats.original_size = decompressed.size();
    stats.compressed_size = input.size();
    stats.compression_ratio = static_cast<double>(stats.compressed_size) / stats.original_size;
    stats.decompression_time_ms = duration_ms(start_time, end_time);
    stats.threads_used = 1;
    
    // Verify integrity if requested
    if (config.verify_integrity) {
        stats.checksum = utils::CRC32::calculate(decompressed);
    }
    
    result.set_data(std::move(decompressed));
    return result;
}

double RLEAlgorithm::estimate_ratio(const ByteVector& input) const {
    if (input.empty()) return 1.0;
    
    // Quick estimate based on run lengths
    size_t runs = 1;
    for (size_t i = 1; i < input.size(); ++i) {
        if (input[i] != input[i-1]) {
            runs++;
        }
    }
    
    // Estimate: each run takes ~2-3 bytes on average
    double estimated_size = runs * 2.5;
    return std::min(1.0, estimated_size / input.size());
}

ByteVector RLEAlgorithm::encode_rle(const ByteVector& input) {
    ByteVector output;
    output.reserve(input.size()); // Conservative estimate
    
    for (size_t i = 0; i < input.size(); ) {
        uint8_t current_byte = input[i];
        size_t run_length = 1;
        
        // Count consecutive identical bytes
        while (i + run_length < input.size() && 
               input[i + run_length] == current_byte && 
               run_length < 255) {
            run_length++;
        }
        
        // Encode the run
        if (run_length >= 3) {
            // Use RLE for runs of 3 or more
            output.push_back(0xFF); // Escape byte
            output.push_back(static_cast<uint8_t>(run_length));
            output.push_back(current_byte);
        } else {
            // Store literal bytes
            for (size_t j = 0; j < run_length; ++j) {
                if (current_byte == 0xFF) {
                    // Escape the escape byte
                    output.push_back(0xFF);
                    output.push_back(0x00);
                } else {
                    output.push_back(current_byte);
                }
            }
        }
        
        i += run_length;
    }
    
    return output;
}

ByteVector RLEAlgorithm::decode_rle(const ByteVector& input) {
    ByteVector output;
    output.reserve(input.size() * 2); // Conservative estimate
    
    for (size_t i = 0; i < input.size(); ) {
        if (input[i] == 0xFF && i + 1 < input.size()) {
            if (input[i + 1] == 0x00) {
                // Escaped 0xFF byte
                output.push_back(0xFF);
                i += 2;
            } else if (i + 2 < input.size()) {
                // RLE sequence
                uint8_t run_length = input[i + 1];
                uint8_t byte_value = input[i + 2];
                
                for (uint8_t j = 0; j < run_length; ++j) {
                    output.push_back(byte_value);
                }
                i += 3;
            } else {
                throw DecompressionException("Corrupted RLE data: incomplete sequence");
            }
        } else {
            // Literal byte
            output.push_back(input[i]);
            i++;
        }
    }
    
    return output;
}

ByteVector RLEAlgorithm::encode_enhanced_rle(const ByteVector& input) {
    ByteVector output;
    output.push_back(0xE1); // Enhanced RLE header
    output.reserve(input.size() + 1);
    
    for (size_t i = 0; i < input.size(); ) {
        uint8_t current_byte = input[i];
        size_t run_length = 1;
        
        // Count consecutive identical bytes
        while (i + run_length < input.size() && 
               input[i + run_length] == current_byte && 
               run_length < 127) {
            run_length++;
        }
        
        if (run_length >= 4) {
            // Encode as run: high bit set + length + byte
            output.push_back(0x80 | static_cast<uint8_t>(run_length));
            output.push_back(current_byte);
        } else {
            // Look ahead for literal sequence
            size_t literal_length = 0;
            size_t j = i;
            
            while (j < input.size() && literal_length < 127) {
                size_t next_run = 1;
                while (j + next_run < input.size() && 
                       input[j + next_run] == input[j] && 
                       next_run < 4) {
                    next_run++;
                }
                
                if (next_run >= 4) break; // Found a run worth encoding
                
                literal_length += next_run;
                j += next_run;
            }
            
            // Encode literal sequence: length + bytes
            output.push_back(static_cast<uint8_t>(literal_length));
            for (size_t k = 0; k < literal_length; ++k) {
                output.push_back(input[i + k]);
            }
            
            i += literal_length;
            continue;
        }
        
        i += run_length;
    }
    
    return output;
}

ByteVector RLEAlgorithm::decode_enhanced_rle(const ByteVector& input) {
    if (input.empty() || input[0] != 0xE1) {
        throw DecompressionException("Invalid enhanced RLE header");
    }
    
    ByteVector output;
    output.reserve(input.size() * 3); // Conservative estimate
    
    for (size_t i = 1; i < input.size(); ) {
        uint8_t control = input[i++];
        
        if (control & 0x80) {
            // Run sequence
            uint8_t run_length = control & 0x7F;
            if (i >= input.size()) {
                throw DecompressionException("Corrupted enhanced RLE data: missing byte value");
            }
            uint8_t byte_value = input[i++];
            
            for (uint8_t j = 0; j < run_length; ++j) {
                output.push_back(byte_value);
            }
        } else {
            // Literal sequence
            uint8_t literal_length = control;
            if (i + literal_length > input.size()) {
                throw DecompressionException("Corrupted enhanced RLE data: incomplete literal sequence");
            }
            
            for (uint8_t j = 0; j < literal_length; ++j) {
                output.push_back(input[i + j]);
            }
            i += literal_length;
        }
    }
    
    return output;
}

double RLEAlgorithm::calculate_entropy(const ByteVector& input) const {
    if (input.empty()) return 0.0;
    
    // Count byte frequencies
    std::unordered_map<uint8_t, size_t> frequencies;
    for (uint8_t byte : input) {
        frequencies[byte]++;
    }
    
    // Calculate Shannon entropy
    double entropy = 0.0;
    double size = static_cast<double>(input.size());
    
    for (const auto& pair : frequencies) {
        double probability = static_cast<double>(pair.second) / size;
        entropy -= probability * std::log2(probability);
    }
    
    return entropy / 8.0; // Normalize to [0,1]
}

} // namespace compressor
