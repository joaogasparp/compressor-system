#ifndef COMPRESSOR_FILE_UTILS_HPP
#define COMPRESSOR_FILE_UTILS_HPP

#include "core/common.hpp"
#include <string>
#include <fstream>

namespace compressor {
namespace utils {

class FileUtils {
public:
    // Read entire file into memory
    static ByteVector read_file(const std::string& filename);
    
    // Write data to file
    static bool write_file(const std::string& filename, const ByteVector& data);
    
    // Get file size without reading
    static size_t get_file_size(const std::string& filename);
    
    // Check if file exists
    static bool file_exists(const std::string& filename);
    
    // Get file extension
    static std::string get_extension(const std::string& filename);
    
    // Create directory if it doesn't exist
    static bool create_directory(const std::string& path);
    
    // Read file in chunks (for large files)
    class FileReader {
    public:
        explicit FileReader(const std::string& filename, size_t chunk_size = 64 * 1024);
        ~FileReader();
        
        bool is_open() const { return file_.is_open(); }
        ByteVector read_chunk();
        bool has_more() const;
        size_t total_size() const { return total_size_; }
        size_t bytes_read() const { return bytes_read_; }
        
    private:
        std::ifstream file_;
        size_t chunk_size_;
        size_t total_size_;
        size_t bytes_read_;
    };
    
    // Write file in chunks
    class FileWriter {
    public:
        explicit FileWriter(const std::string& filename);
        ~FileWriter();
        
        bool is_open() const { return file_.is_open(); }
        bool write_chunk(const ByteVector& data);
        size_t bytes_written() const { return bytes_written_; }
        
    private:
        std::ofstream file_;
        size_t bytes_written_;
    };
};

} // namespace utils
} // namespace compressor

#endif // COMPRESSOR_FILE_UTILS_HPP
