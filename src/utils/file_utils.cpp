#include "utils/file_utils.hpp"
#include <fstream>
#include <sys/stat.h>
#include <stdexcept>

namespace compressor {
namespace utils {

ByteVector FileUtils::read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Read data
    ByteVector data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    
    if (!file) {
        throw std::runtime_error("Error reading file: " + filename);
    }
    
    return data;
}

bool FileUtils::write_file(const std::string& filename, const ByteVector& data) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return file.good();
}

size_t FileUtils::get_file_size(const std::string& filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : 0;
}

bool FileUtils::file_exists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

std::string FileUtils::get_extension(const std::string& filename) {
    size_t dot_pos = filename.rfind('.');
    if (dot_pos == std::string::npos) {
        return "";
    }
    return filename.substr(dot_pos + 1);
}

bool FileUtils::create_directory(const std::string& path) {
    return mkdir(path.c_str(), 0755) == 0;
}

// FileReader implementation
FileUtils::FileReader::FileReader(const std::string& filename, size_t chunk_size)
    : file_(filename, std::ios::binary), chunk_size_(chunk_size), bytes_read_(0) {
    
    if (file_.is_open()) {
        file_.seekg(0, std::ios::end);
        total_size_ = file_.tellg();
        file_.seekg(0, std::ios::beg);
    } else {
        total_size_ = 0;
    }
}

FileUtils::FileReader::~FileReader() {
    if (file_.is_open()) {
        file_.close();
    }
}

ByteVector FileUtils::FileReader::read_chunk() {
    if (!has_more()) {
        return ByteVector();
    }
    
    size_t to_read = std::min(chunk_size_, total_size_ - bytes_read_);
    ByteVector chunk(to_read);
    
    file_.read(reinterpret_cast<char*>(chunk.data()), to_read);
    size_t actually_read = file_.gcount();
    
    if (actually_read != to_read) {
        chunk.resize(actually_read);
    }
    
    bytes_read_ += actually_read;
    return chunk;
}

bool FileUtils::FileReader::has_more() const {
    return bytes_read_ < total_size_ && file_.good();
}

// FileWriter implementation
FileUtils::FileWriter::FileWriter(const std::string& filename)
    : file_(filename, std::ios::binary), bytes_written_(0) {
}

FileUtils::FileWriter::~FileWriter() {
    if (file_.is_open()) {
        file_.close();
    }
}

bool FileUtils::FileWriter::write_chunk(const ByteVector& data) {
    if (!file_.is_open() || data.empty()) {
        return false;
    }
    
    file_.write(reinterpret_cast<const char*>(data.data()), data.size());
    if (file_.good()) {
        bytes_written_ += data.size();
        return true;
    }
    
    return false;
}

} // namespace utils
} // namespace compressor
