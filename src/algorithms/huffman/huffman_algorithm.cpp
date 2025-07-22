#include "algorithms/huffman/huffman_algorithm.hpp"
#include "utils/crc.hpp"
#include <cmath>
#include <algorithm>

namespace compressor {

// Comparator for priority queue (min-heap)
struct NodeComparator {
    bool operator()(const std::unique_ptr<HuffmanNode>& a, const std::unique_ptr<HuffmanNode>& b) {
        if (a->frequency != b->frequency) {
            return a->frequency > b->frequency;
        }
        // Tie-breaker: prefer leaves over internal nodes for canonical codes
        return a->is_leaf() < b->is_leaf();
    }
};

AlgorithmInfo HuffmanAlgorithm::get_info() const {
    return AlgorithmInfo(
        "huffman",
        "Huffman Coding - Optimal prefix coding for symbol compression",
        false, // Basic implementation doesn't support parallel processing
        4096   // Minimum block size for reasonable frequency analysis
    );
}

CompressionResult HuffmanAlgorithm::compress(const ByteVector& input, const CompressionConfig& config) {
    if (input.empty()) {
        return CompressionResult(false, "Input data is empty");
    }
    
    CompressionResult result(true);
    auto& stats = result.stats();
    
    stats.original_size = input.size();
    if (config.verify_integrity) {
        stats.checksum = utils::CRC32::calculate(input);
    }
    
    auto start_time = now();
    
    // Count byte frequencies
    std::unordered_map<uint8_t, size_t> frequencies;
    for (uint8_t byte : input) {
        frequencies[byte]++;
    }
    
    // Handle special case: only one unique byte
    if (frequencies.size() == 1) {
        ByteVector compressed;
        compressed.push_back(0x01); // Special header for single byte
        compressed.push_back(frequencies.begin()->first); // The byte
        
        // Store the count (up to 4 bytes)
        size_t count = input.size();
        compressed.push_back((count >> 24) & 0xFF);
        compressed.push_back((count >> 16) & 0xFF);
        compressed.push_back((count >> 8) & 0xFF);
        compressed.push_back(count & 0xFF);
        
        auto end_time = now();
        stats.compressed_size = compressed.size();
        stats.compression_ratio = static_cast<double>(stats.compressed_size) / stats.original_size;
        stats.compression_time_ms = duration_ms(start_time, end_time);
        stats.threads_used = 1;
        
        result.set_data(std::move(compressed));
        return result;
    }
    
    // Build Huffman tree
    auto tree = build_tree(frequencies);
    auto codes = generate_codes(tree.get());
    
    // Serialize the tree
    ByteVector tree_data = serialize_tree(tree.get());
    
    // Compress data
    ByteVector compressed;
    compressed.push_back(0x02); // Normal Huffman header
    
    // Store tree size (2 bytes)
    compressed.push_back((tree_data.size() >> 8) & 0xFF);
    compressed.push_back(tree_data.size() & 0xFF);
    
    // Store tree
    compressed.insert(compressed.end(), tree_data.begin(), tree_data.end());
    
    // Store original size (4 bytes)
    size_t original_size = input.size();
    compressed.push_back((original_size >> 24) & 0xFF);
    compressed.push_back((original_size >> 16) & 0xFF);
    compressed.push_back((original_size >> 8) & 0xFF);
    compressed.push_back(original_size & 0xFF);
    
    // Encode data
    BitWriter writer(compressed);
    for (uint8_t byte : input) {
        const auto& code = codes[byte];
        writer.write_bits(code.code, code.length);
    }
    writer.flush();
    
    auto end_time = now();
    
    stats.compressed_size = compressed.size();
    stats.compression_ratio = static_cast<double>(stats.compressed_size) / stats.original_size;
    stats.compression_time_ms = duration_ms(start_time, end_time);
    stats.threads_used = 1;
    
    result.set_data(std::move(compressed));
    
    if (config.verbose) {
        printf("Huffman compression: %.2f%% (%zu unique bytes)\n", 
               stats.compression_ratio * 100.0, frequencies.size());
    }
    
    return result;
}

CompressionResult HuffmanAlgorithm::decompress(const ByteVector& input, const CompressionConfig& config) {
    if (input.empty()) {
        return CompressionResult(false, "Input data is empty");
    }
    
    CompressionResult result(true);
    auto& stats = result.stats();
    
    auto start_time = now();
    
    try {
        ByteVector decompressed;
        
        if (input[0] == 0x01) {
            // Special case: single byte
            if (input.size() < 6) {
                throw DecompressionException("Invalid single-byte Huffman data");
            }
            
            uint8_t byte_value = input[1];
            size_t count = (static_cast<size_t>(input[2]) << 24) |
                          (static_cast<size_t>(input[3]) << 16) |
                          (static_cast<size_t>(input[4]) << 8) |
                          static_cast<size_t>(input[5]);
            
            decompressed.assign(count, byte_value);
        } else if (input[0] == 0x02) {
            // Normal Huffman data
            if (input.size() < 7) {
                throw DecompressionException("Invalid Huffman data header");
            }
            
            // Read tree size
            size_t tree_size = (static_cast<size_t>(input[1]) << 8) | input[2];
            if (3 + tree_size + 4 > input.size()) {
                throw DecompressionException("Invalid tree size");
            }
            
            // Deserialize tree
            size_t offset = 3;
            auto tree = deserialize_tree(input, offset);
            
            // Read original size
            size_t original_size = (static_cast<size_t>(input[offset]) << 24) |
                                  (static_cast<size_t>(input[offset + 1]) << 16) |
                                  (static_cast<size_t>(input[offset + 2]) << 8) |
                                  static_cast<size_t>(input[offset + 3]);
            offset += 4;
            
            // Decode data
            ByteVector compressed_data(input.begin() + offset, input.end());
            BitReader reader(compressed_data);
            
            decompressed.reserve(original_size);
            
            for (size_t i = 0; i < original_size; ++i) {
                const HuffmanNode* current = tree.get();
                
                while (!current->is_leaf()) {
                    if (!reader.has_more()) {
                        throw DecompressionException("Unexpected end of compressed data");
                    }
                    
                    uint32_t bit = reader.read_bits(1);
                    current = bit ? current->right.get() : current->left.get();
                    
                    if (!current) {
                        throw DecompressionException("Invalid Huffman tree traversal");
                    }
                }
                
                decompressed.push_back(current->byte);
            }
        } else {
            throw DecompressionException("Unknown Huffman format");
        }
        
        auto end_time = now();
        
        stats.original_size = decompressed.size();
        stats.compressed_size = input.size();
        stats.compression_ratio = static_cast<double>(stats.compressed_size) / stats.original_size;
        stats.decompression_time_ms = duration_ms(start_time, end_time);
        stats.threads_used = 1;
        
        if (config.verify_integrity) {
            stats.checksum = utils::CRC32::calculate(decompressed);
        }
        
        result.set_data(std::move(decompressed));
        
    } catch (const std::exception& e) {
        return CompressionResult(false, "Decompression failed: " + std::string(e.what()));
    }
    
    return result;
}

double HuffmanAlgorithm::estimate_ratio(const ByteVector& input) const {
    if (input.empty()) return 1.0;
    
    // Count frequencies
    std::unordered_map<uint8_t, size_t> frequencies;
    for (uint8_t byte : input) {
        frequencies[byte]++;
    }
    
    // Calculate theoretical Huffman compression size
    double entropy = calculate_entropy(frequencies, input.size());
    
    // Huffman achieves close to entropy, add overhead for tree storage
    double tree_overhead = frequencies.size() * 9; // Rough estimate
    double theoretical_bits = entropy * input.size() * 8 + tree_overhead;
    
    return std::min(1.0, theoretical_bits / (input.size() * 8));
}

std::unique_ptr<HuffmanNode> HuffmanAlgorithm::build_tree(const std::unordered_map<uint8_t, size_t>& frequencies) {
    std::priority_queue<std::unique_ptr<HuffmanNode>, 
                       std::vector<std::unique_ptr<HuffmanNode>>, 
                       NodeComparator> pq;
    
    // Create leaf nodes
    for (const auto& pair : frequencies) {
        pq.push(std::make_unique<HuffmanNode>(pair.first, pair.second));
    }
    
    // Build tree bottom-up
    while (pq.size() > 1) {
        auto right = std::move(const_cast<std::unique_ptr<HuffmanNode>&>(pq.top()));
        pq.pop();
        auto left = std::move(const_cast<std::unique_ptr<HuffmanNode>&>(pq.top()));
        pq.pop();
        
        auto parent = std::make_unique<HuffmanNode>(
            left->frequency + right->frequency,
            std::move(left),
            std::move(right)
        );
        
        pq.push(std::move(parent));
    }
    
    return std::move(const_cast<std::unique_ptr<HuffmanNode>&>(pq.top()));
}

std::unordered_map<uint8_t, HuffmanCode> HuffmanAlgorithm::generate_codes(const HuffmanNode* root) {
    std::unordered_map<uint8_t, HuffmanCode> codes;
    
    if (root->is_leaf()) {
        // Special case: single symbol gets code "0"
        codes[root->byte] = HuffmanCode(0, 1);
    } else {
        generate_codes_recursive(root, 0, 0, codes);
    }
    
    return codes;
}

void HuffmanAlgorithm::generate_codes_recursive(const HuffmanNode* node, uint32_t code, uint8_t depth,
                                               std::unordered_map<uint8_t, HuffmanCode>& codes) {
    if (node->is_leaf()) {
        codes[node->byte] = HuffmanCode(code, depth);
        return;
    }
    
    if (node->left) {
        generate_codes_recursive(node->left.get(), code << 1, depth + 1, codes);
    }
    if (node->right) {
        generate_codes_recursive(node->right.get(), (code << 1) | 1, depth + 1, codes);
    }
}

ByteVector HuffmanAlgorithm::serialize_tree(const HuffmanNode* root) {
    ByteVector data;
    
    if (!root) return data;
    
    if (root->is_leaf()) {
        data.push_back(1); // Leaf marker
        data.push_back(root->byte);
    } else {
        data.push_back(0); // Internal node marker
        
        auto left_data = serialize_tree(root->left.get());
        auto right_data = serialize_tree(root->right.get());
        
        data.insert(data.end(), left_data.begin(), left_data.end());
        data.insert(data.end(), right_data.begin(), right_data.end());
    }
    
    return data;
}

std::unique_ptr<HuffmanNode> HuffmanAlgorithm::deserialize_tree(const ByteVector& data, size_t& offset) {
    if (offset >= data.size()) {
        throw DecompressionException("Corrupted tree data");
    }
    
    uint8_t marker = data[offset++];
    
    if (marker == 1) {
        // Leaf node
        if (offset >= data.size()) {
            throw DecompressionException("Corrupted leaf node data");
        }
        uint8_t byte_value = data[offset++];
        return std::make_unique<HuffmanNode>(byte_value, 0);
    } else {
        // Internal node
        auto left = deserialize_tree(data, offset);
        auto right = deserialize_tree(data, offset);
        return std::make_unique<HuffmanNode>(0, std::move(left), std::move(right));
    }
}

void HuffmanAlgorithm::BitWriter::write_bits(uint32_t value, uint8_t count) {
    while (count > 0) {
        uint8_t bits_to_write = std::min(count, static_cast<uint8_t>(8 - bits_used_));
        uint8_t shift = count - bits_to_write;
        uint8_t bits = (value >> shift) & ((1 << bits_to_write) - 1);
        
        current_byte_ |= bits << (8 - bits_used_ - bits_to_write);
        bits_used_ += bits_to_write;
        count -= bits_to_write;
        
        if (bits_used_ == 8) {
            output_.push_back(current_byte_);
            current_byte_ = 0;
            bits_used_ = 0;
        }
    }
}

void HuffmanAlgorithm::BitWriter::flush() {
    if (bits_used_ > 0) {
        output_.push_back(current_byte_);
        current_byte_ = 0;
        bits_used_ = 0;
    }
}

uint32_t HuffmanAlgorithm::BitReader::read_bits(uint8_t count) {
    uint32_t result = 0;
    
    while (count > 0) {
        if (bits_available_ == 0) {
            if (position_ >= input_.size()) {
                throw DecompressionException("Unexpected end of bit stream");
            }
            current_byte_ = input_[position_++];
            bits_available_ = 8;
        }
        
        uint8_t bits_to_read = std::min(count, bits_available_);
        uint8_t shift = bits_available_ - bits_to_read;
        uint8_t mask = (1 << bits_to_read) - 1;
        uint8_t bits = (current_byte_ >> shift) & mask;
        
        result = (result << bits_to_read) | bits;
        count -= bits_to_read;
        bits_available_ -= bits_to_read;
        current_byte_ &= (1 << bits_available_) - 1;
    }
    
    return result;
}

bool HuffmanAlgorithm::BitReader::has_more() const {
    return position_ < input_.size() || bits_available_ > 0;
}

double HuffmanAlgorithm::calculate_entropy(const std::unordered_map<uint8_t, size_t>& frequencies, size_t total_size) const {
    double entropy = 0.0;
    
    for (const auto& pair : frequencies) {
        double probability = static_cast<double>(pair.second) / total_size;
        entropy -= probability * std::log2(probability);
    }
    
    return entropy / 8.0; // Normalize to bytes
}

} // namespace compressor
