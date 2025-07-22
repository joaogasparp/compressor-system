#ifndef COMPRESSOR_CRC_HPP
#define COMPRESSOR_CRC_HPP

#include "core/common.hpp"
#include <cstdint>

namespace compressor {
namespace utils {

class CRC32 {
public:
    CRC32();
    
    // Calculate CRC32 for data
    static uint32_t calculate(const ByteVector& data);
    static uint32_t calculate(const uint8_t* data, size_t length);
    
    // Incremental CRC calculation
    void reset();
    void update(const ByteVector& data);
    void update(const uint8_t* data, size_t length);
    uint32_t finalize() const;
    
private:
    static uint32_t crc_table_[256];
    static bool table_initialized_;
    static void init_table();
    
    uint32_t crc_;
};

} // namespace utils
} // namespace compressor

#endif // COMPRESSOR_CRC_HPP
