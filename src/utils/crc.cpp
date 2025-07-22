#include "utils/crc.hpp"

namespace compressor {
namespace utils {

uint32_t CRC32::crc_table_[256];
bool CRC32::table_initialized_ = false;

CRC32::CRC32() : crc_(0xFFFFFFFF) {
    if (!table_initialized_) {
        init_table();
    }
}

void CRC32::init_table() {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320UL;
            } else {
                crc >>= 1;
            }
        }
        crc_table_[i] = crc;
    }
    table_initialized_ = true;
}

uint32_t CRC32::calculate(const ByteVector& data) {
    return calculate(data.data(), data.size());
}

uint32_t CRC32::calculate(const uint8_t* data, size_t length) {
    CRC32 crc;
    crc.update(data, length);
    return crc.finalize();
}

void CRC32::reset() {
    crc_ = 0xFFFFFFFF;
}

void CRC32::update(const ByteVector& data) {
    update(data.data(), data.size());
}

void CRC32::update(const uint8_t* data, size_t length) {
    if (!table_initialized_) {
        init_table();
    }
    
    for (size_t i = 0; i < length; i++) {
        crc_ = crc_table_[(crc_ ^ data[i]) & 0xFF] ^ (crc_ >> 8);
    }
}

uint32_t CRC32::finalize() const {
    return crc_ ^ 0xFFFFFFFF;
}

} // namespace utils
} // namespace compressor
