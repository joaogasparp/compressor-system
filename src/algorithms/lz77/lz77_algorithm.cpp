#include "algorithms/lz77/lz77_algorithm.hpp"
#include "utils/crc.hpp"
#include <algorithm>
#include <cstring>

namespace compressor {

AlgorithmInfo LZ77Algorithm::get_info() const {
    return AlgorithmInfo(
        "lz77",
        "LZ77 Dictionary Compression - Efficient for files with repeated patterns",
        false, // Basic implementation doesn't support parallel processing
        8192   // Minimum block size for effective dictionary building
    );
}

CompressionResult LZ77Algorithm::compress(const ByteVector& input, const CompressionConfig& config) {
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
    
    // Find all matches
    std::vector<LZ77Match> matches;
    matches.reserve(input.size() / 4); // Conservative estimate
    
    HashSearch searcher;
    
    for (size_t i = 0; i < input.size(); ) {
        // Update hash table with current position
        if (i >= 2) {
            searcher.update(input, i - 2);
        }
        
        // Find longest match
        LZ77Match match = searcher.find_match(input, i);
        
        if (match.is_literal()) {
            match.next_char = input[i];
            matches.push_back(match);
            i++;
        } else {
            // Ensure we don't go beyond input bounds
            if (i + match.length < input.size()) {
                match.next_char = input[i + match.length];
            } else {
                match.next_char = 0;
            }
            matches.push_back(match);
            
            // Update hash table for all positions in the match
            for (uint8_t j = 0; j < match.length && i + j + 2 < input.size(); ++j) {
                searcher.update(input, i + j);
            }
            
            i += match.length + 1;
        }
    }
    
    // Encode matches
    ByteVector compressed = encode_matches(matches);
    
    auto end_time = now();
    
    stats.compressed_size = compressed.size();
    stats.compression_ratio = static_cast<double>(stats.compressed_size) / stats.original_size;
    stats.compression_time_ms = duration_ms(start_time, end_time);
    stats.threads_used = 1;
    
    result.set_data(std::move(compressed));
    
    if (config.verbose) {
        printf("LZ77 compression: %.2f%% (%zu matches)\n", 
               stats.compression_ratio * 100.0, matches.size());
    }
    
    return result;
}

CompressionResult LZ77Algorithm::decompress(const ByteVector& input, const CompressionConfig& config) {
    if (input.empty()) {
        return CompressionResult(false, "Input data is empty");
    }
    
    CompressionResult result(true);
    auto& stats = result.stats();
    
    auto start_time = now();
    
    try {
        // Decode matches
        auto matches = decode_matches(input);
        
        // Reconstruct original data
        ByteVector decompressed;
        decompressed.reserve(input.size() * 3); // Conservative estimate
        
        for (const auto& match : matches) {
            if (match.is_literal()) {
                decompressed.push_back(match.next_char);
            } else {
                // Copy from sliding window
                size_t start_pos = decompressed.size() - match.distance;
                
                for (uint8_t i = 0; i < match.length; ++i) {
                    if (start_pos + i >= decompressed.size()) {
                        throw DecompressionException("Invalid LZ77 match distance");
                    }
                    decompressed.push_back(decompressed[start_pos + i]);
                }
                
                if (match.next_char != 0 || !matches.empty()) {
                    decompressed.push_back(match.next_char);
                }
            }
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

double LZ77Algorithm::estimate_ratio(const ByteVector& input) const {
    if (input.empty()) return 1.0;
    
    // Quick estimation based on local redundancy
    size_t matches = 0;
    size_t window_start = 0;
    
    for (size_t i = MIN_MATCH_LENGTH; i < input.size(); ++i) {
        window_start = (i >= WINDOW_SIZE) ? i - WINDOW_SIZE : 0;
        
        // Look for matches in a small window
        for (size_t j = window_start; j < i - MIN_MATCH_LENGTH + 1; ++j) {
            if (input[i] == input[j] && input[i+1] == input[j+1] && input[i+2] == input[j+2]) {
                matches++;
                break;
            }
        }
    }
    
    // Estimate compression: each match saves ~2-3 bytes on average
    double saved_bytes = matches * 2.5;
    return std::max(0.1, 1.0 - saved_bytes / input.size());
}

LZ77Match LZ77Algorithm::find_longest_match(const ByteVector& input, size_t position) {
    if (position + MIN_MATCH_LENGTH > input.size()) {
        return LZ77Match(); // Return literal
    }
    
    size_t window_start = (position >= WINDOW_SIZE) ? position - WINDOW_SIZE : 0;
    size_t best_distance = 0;
    uint8_t best_length = 0;
    
    // Search in the sliding window
    for (size_t i = window_start; i < position; ++i) {
        uint8_t match_length = 0;
        size_t max_length = std::min(LOOKAHEAD_SIZE, input.size() - position);
        
        // Count matching bytes
        while (match_length < max_length && 
               input[i + match_length] == input[position + match_length]) {
            match_length++;
        }
        
        // Update best match if this is longer
        if (match_length >= MIN_MATCH_LENGTH && match_length > best_length) {
            best_length = match_length;
            best_distance = position - i;
        }
    }
    
    if (best_length >= MIN_MATCH_LENGTH) {
        uint8_t next_char = (position + best_length < input.size()) ? input[position + best_length] : 0;
        return LZ77Match(best_distance, best_length, next_char);
    }
    
    return LZ77Match(); // Return literal
}

ByteVector LZ77Algorithm::encode_matches(const std::vector<LZ77Match>& matches) {
    ByteVector encoded;
    encoded.reserve(matches.size() * 4); // Conservative estimate
    
    // Header: LZ77 signature and match count
    encoded.push_back('L');
    encoded.push_back('Z');
    encoded.push_back('7');
    encoded.push_back('7');
    
    uint32_t match_count = matches.size();
    encoded.push_back((match_count >> 24) & 0xFF);
    encoded.push_back((match_count >> 16) & 0xFF);
    encoded.push_back((match_count >> 8) & 0xFF);
    encoded.push_back(match_count & 0xFF);
    
    // Encode each match
    for (const auto& match : matches) {
        if (match.is_literal()) {
            encoded.push_back(0x00); // Literal marker
            encoded.push_back(match.next_char);
        } else {
            encoded.push_back(0x01); // Match marker
            encoded.push_back((match.distance >> 8) & 0xFF);
            encoded.push_back(match.distance & 0xFF);
            encoded.push_back(match.length);
            encoded.push_back(match.next_char);
        }
    }
    
    return encoded;
}

std::vector<LZ77Match> LZ77Algorithm::decode_matches(const ByteVector& encoded) {
    if (encoded.size() < 8) {
        throw DecompressionException("Invalid LZ77 header");
    }
    
    // Check signature
    if (encoded[0] != 'L' || encoded[1] != 'Z' || encoded[2] != '7' || encoded[3] != '7') {
        throw DecompressionException("Invalid LZ77 signature");
    }
    
    // Read match count
    uint32_t match_count = (static_cast<uint32_t>(encoded[4]) << 24) |
                          (static_cast<uint32_t>(encoded[5]) << 16) |
                          (static_cast<uint32_t>(encoded[6]) << 8) |
                          static_cast<uint32_t>(encoded[7]);
    
    std::vector<LZ77Match> matches;
    matches.reserve(match_count);
    
    size_t offset = 8;
    for (uint32_t i = 0; i < match_count; ++i) {
        if (offset >= encoded.size()) {
            throw DecompressionException("Unexpected end of LZ77 data");
        }
        
        uint8_t marker = encoded[offset++];
        
        if (marker == 0x00) {
            // Literal
            if (offset >= encoded.size()) {
                throw DecompressionException("Incomplete literal in LZ77 data");
            }
            LZ77Match match;
            match.next_char = encoded[offset++];
            matches.push_back(match);
        } else if (marker == 0x01) {
            // Match
            if (offset + 3 >= encoded.size()) {
                throw DecompressionException("Incomplete match in LZ77 data");
            }
            
            uint16_t distance = (static_cast<uint16_t>(encoded[offset]) << 8) | encoded[offset + 1];
            uint8_t length = encoded[offset + 2];
            uint8_t next_char = encoded[offset + 3];
            offset += 4;
            
            matches.emplace_back(distance, length, next_char);
        } else {
            throw DecompressionException("Invalid LZ77 marker");
        }
    }
    
    return matches;
}

// HashSearch implementation
LZ77Algorithm::HashSearch::HashSearch() : hash_table_(HASH_SIZE) {
}

void LZ77Algorithm::HashSearch::update(const ByteVector& input, size_t position) {
    if (position + 2 >= input.size()) return;
    
    uint32_t hash = hash3(input[position], input[position + 1], input[position + 2]);
    auto& chain = hash_table_[hash];
    
    // Add position to hash chain
    chain.push_back(position);
    
    // Limit chain length to prevent excessive search times
    if (chain.size() > 16) {
        chain.erase(chain.begin());
    }
}

LZ77Match LZ77Algorithm::HashSearch::find_match(const ByteVector& input, size_t position) {
    if (position + MIN_MATCH_LENGTH > input.size()) {
        return LZ77Match();
    }
    
    uint32_t hash = hash3(input[position], input[position + 1], input[position + 2]);
    const auto& chain = hash_table_[hash];
    
    uint16_t best_distance = 0;
    uint8_t best_length = 0;
    
    // Search hash chain for matches
    for (auto it = chain.rbegin(); it != chain.rend(); ++it) {
        size_t candidate_pos = *it;
        
        if (candidate_pos >= position) continue;
        
        uint16_t distance = position - candidate_pos;
        if (distance > WINDOW_SIZE) break;
        
        // Count matching bytes
        uint8_t match_length = 0;
        size_t max_length = std::min(LOOKAHEAD_SIZE, input.size() - position);
        
        while (match_length < max_length && 
               input[candidate_pos + match_length] == input[position + match_length]) {
            match_length++;
        }
        
        if (match_length > best_length) {
            best_length = match_length;
            best_distance = distance;
        }
    }
    
    if (best_length >= MIN_MATCH_LENGTH) {
        uint8_t next_char = (position + best_length < input.size()) ? input[position + best_length] : 0;
        return LZ77Match(best_distance, best_length, next_char);
    }
    
    return LZ77Match();
}

void LZ77Algorithm::HashSearch::clear() {
    for (auto& chain : hash_table_) {
        chain.clear();
    }
}

uint32_t LZ77Algorithm::HashSearch::hash3(uint8_t a, uint8_t b, uint8_t c) const {
    return ((static_cast<uint32_t>(a) << 16) | 
            (static_cast<uint32_t>(b) << 8) | 
            static_cast<uint32_t>(c)) & HASH_MASK;
}

} // namespace compressor
