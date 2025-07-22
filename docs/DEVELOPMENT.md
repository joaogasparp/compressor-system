# Development Guide - Compressor System

## Development Environment Setup

### Required Tools
- **C++ Compiler**: GCC 7+ or Clang 5+ with C++17 support
- **Build System**: CMake 3.16+
- **Debugger**: GDB or LLDB
- **Static Analysis**: clang-tidy, cppcheck
- **Memory Analysis**: Valgrind, AddressSanitizer
- **Profiling**: perf, gprof, or Intel VTune

### IDE Configuration
For **Visual Studio Code**:
```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/src",
                "/usr/include/c++/**"
            ],
            "defines": [],
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "linux-gcc-x64"
        }
    ]
}
```

## Project Structure

```
compressor-system/
├── src/                    # Source code
│   ├── core/              # Core interfaces and utilities
│   ├── algorithms/        # Compression algorithm implementations
│   ├── benchmark/         # Performance measurement framework
│   ├── cli/              # Command-line interface
│   ├── utils/            # Utility functions
│   └── main.cpp          # Application entry point
├── test/                  # Test files and data
│   └── sample-files/     # Sample data for testing
├── docs/                 # Documentation
├── build/                # Build directory (generated)
├── CMakeLists.txt        # CMake configuration
└── README.md             # Project overview
```

## Adding a New Algorithm

### 1. Create Algorithm Class

Create files in `src/algorithms/your_algorithm/`:

**your_algorithm.hpp**:
```cpp
#ifndef COMPRESSOR_YOUR_ALGORITHM_HPP
#define COMPRESSOR_YOUR_ALGORITHM_HPP

#include "core/algorithm.hpp"

namespace compressor {

class YourAlgorithm : public Algorithm {
public:
    AlgorithmInfo get_info() const override;
    CompressionResult compress(const ByteVector& input, 
                             const CompressionConfig& config) override;
    CompressionResult decompress(const ByteVector& input,
                               const CompressionConfig& config) override;
    double estimate_ratio(const ByteVector& input) const override;

private:
    // Your private implementation methods
};

} // namespace compressor

#endif
```

**your_algorithm.cpp**:
```cpp
#include "algorithms/your_algorithm/your_algorithm.hpp"
#include "utils/crc.hpp"

namespace compressor {

AlgorithmInfo YourAlgorithm::get_info() const {
    return AlgorithmInfo(
        "your_algo",
        "Description of your algorithm",
        false, // supports_parallel
        1024   // min_block_size
    );
}

CompressionResult YourAlgorithm::compress(const ByteVector& input, 
                                        const CompressionConfig& config) {
    // Implementation
}

CompressionResult YourAlgorithm::decompress(const ByteVector& input,
                                          const CompressionConfig& config) {
    // Implementation
}

double YourAlgorithm::estimate_ratio(const ByteVector& input) const {
    // Quick estimation without actual compression
}

} // namespace compressor
```

### 2. Register Algorithm

Add to `src/core/algorithm.cpp`:
```cpp
#include "algorithms/your_algorithm/your_algorithm.hpp"

// In algorithm_registry map:
{"your_algo", []() { return std::make_unique<YourAlgorithm>(); }}
```

### 3. Update CMakeLists.txt

The current CMakeLists.txt automatically includes all `.cpp` files, so no changes needed.

## Coding Standards

### C++ Style Guidelines

**Naming Conventions**:
- Classes: `PascalCase` (e.g., `CompressionResult`)
- Functions/variables: `snake_case` (e.g., `compress_data`)
- Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_BLOCK_SIZE`)
- Member variables: `trailing_underscore_` (e.g., `data_`)

**Code Structure**:
```cpp
// Good: Clear, documented function
CompressionResult compress_block(const ByteVector& input, size_t block_size) {
    // Validate input
    if (input.empty()) {
        return CompressionResult(false, "Empty input");
    }
    
    // Time the operation
    auto start_time = now();
    
    // Perform compression
    ByteVector compressed = perform_compression(input, block_size);
    
    // Create result
    CompressionResult result(true);
    result.set_data(std::move(compressed));
    result.stats().compression_time_ms = duration_ms(start_time, now());
    
    return result;
}
```

**Error Handling**:
- Use exceptions for unrecoverable errors
- Return `CompressionResult` with error messages for algorithm failures
- Validate inputs at public API boundaries
- Use RAII for resource management

### Memory Management

**Best Practices**:
- Prefer stack allocation over heap when possible
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) for dynamic allocation
- Move semantics for large data transfers
- Avoid raw pointers except for non-owning references

**Example**:
```cpp
// Good: Move semantics, RAII
std::unique_ptr<Algorithm> create_algorithm(const std::string& name) {
    auto algo = std::make_unique<HuffmanAlgorithm>();
    return algo; // Automatic move
}

// Good: Move large data
ByteVector process_data(ByteVector input) {
    ByteVector result = expensive_operation(std::move(input));
    return result; // Move, not copy
}
```

## Testing Framework

### Unit Testing Structure

Create test files in `test/` directory:

**test_your_algorithm.cpp**:
```cpp
#include "algorithms/your_algorithm/your_algorithm.hpp"
#include <cassert>
#include <iostream>

void test_basic_compression() {
    YourAlgorithm algo;
    ByteVector input = {1, 2, 3, 4, 5};
    
    auto result = algo.compress(input);
    assert(result.is_success());
    
    auto decompressed = algo.decompress(result.data());
    assert(decompressed.is_success());
    assert(decompressed.data() == input);
    
    std::cout << "Basic compression test passed\n";
}

void test_empty_input() {
    YourAlgorithm algo;
    ByteVector empty;
    
    auto result = algo.compress(empty);
    // Should handle gracefully, either compress or return error
    
    std::cout << "Empty input test passed\n";
}

int main() {
    test_basic_compression();
    test_empty_input();
    return 0;
}
```

### Benchmark Testing

Test performance characteristics:
```cpp
void benchmark_your_algorithm() {
    YourAlgorithm algo;
    
    // Test different data sizes
    for (size_t size : {1024, 16384, 65536, 1048576}) {
        ByteVector test_data(size, 'A'); // Simple repetitive data
        
        auto start = std::chrono::high_resolution_clock::now();
        auto result = algo.compress(test_data);
        auto end = std::chrono::high_resolution_clock::now();
        
        double time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        double ratio = static_cast<double>(result.data().size()) / test_data.size();
        
        std::cout << "Size: " << size << ", Ratio: " << ratio 
                  << ", Time: " << time_ms << "ms\n";
    }
}
```

## Debugging and Profiling

### Debug Builds

Enable detailed debugging:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-g -O0 -fsanitize=address"
```

### Memory Debugging

**AddressSanitizer**:
```bash
export ASAN_OPTIONS=detect_leaks=1:check_initialization_order=1
./compressor compress -f test.txt -a your_algo
```

**Valgrind**:
```bash
valgrind --tool=memcheck --leak-check=full ./compressor compress -f test.txt -a your_algo
```

### Performance Profiling

**Basic timing**:
```bash
time ./compressor benchmark -f large_file.txt
```

**Detailed profiling with perf**:
```bash
perf record ./compressor benchmark -f large_file.txt
perf report
```

**Call graph analysis**:
```bash
valgrind --tool=callgrind ./compressor compress -f test.txt -a your_algo
kcachegrind callgrind.out.*
```

## Performance Optimization

### Profiling-Guided Development

1. **Measure first**: Use profilers to identify bottlenecks
2. **Optimize hot paths**: Focus on frequently called functions
3. **Memory access patterns**: Improve cache locality
4. **Algorithm complexity**: Reduce computational complexity

### Common Optimization Patterns

**Loop optimization**:
```cpp
// Good: Cache-friendly access pattern
for (size_t i = 0; i < height; ++i) {
    for (size_t j = 0; j < width; ++j) {
        process_pixel(image[i * width + j]);
    }
}

// Good: Reduce function call overhead
const auto end = container.end();
for (auto it = container.begin(); it != end; ++it) {
    process(*it);
}
```

**Memory allocation optimization**:
```cpp
// Good: Reserve capacity to avoid reallocations
ByteVector result;
result.reserve(estimated_size);

// Good: Reuse buffers when possible
class Compressor {
    ByteVector work_buffer_; // Reuse across calls
public:
    CompressionResult compress(const ByteVector& input) {
        work_buffer_.clear();
        work_buffer_.reserve(input.size());
        // Use work_buffer_ for processing
    }
};
```

## Documentation Standards

### Code Documentation

Use clear, descriptive comments:
```cpp
/**
 * Compresses input data using the specified algorithm.
 * 
 * @param input Raw data to compress
 * @param config Compression parameters (block size, threads, etc.)
 * @return Compression result with statistics and compressed data
 * 
 * @throws CompressionException if compression fails due to algorithm error
 * 
 * Time complexity: O(n) where n is input size
 * Space complexity: O(n) for temporary buffers
 */
CompressionResult compress(const ByteVector& input, const CompressionConfig& config);
```

### Algorithm Documentation

Document algorithm specifics:
```cpp
/**
 * Custom Hybrid Compression Algorithm
 * 
 * Strategy:
 * 1. Analyze input data characteristics (entropy, repetition)
 * 2. Classify into blocks based on data type
 * 3. Apply optimal algorithm per block:
 *    - Low entropy: RLE
 *    - High repetition: LZ77
 *    - Random data: Huffman
 *    - Mixed: Try all, select best
 * 
 * Performance characteristics:
 * - Best for: Mixed data types, adaptive scenarios
 * - Time: O(n) with analysis overhead
 * - Space: O(block_size) additional memory
 * 
 * Trade-offs:
 * - Higher CPU usage for analysis
 * - Better compression ratios on varied data
 * - Slight overhead for small files
 */
class HybridAlgorithm : public Algorithm {
    // Implementation...
};
```

## Release Process

### Version Management

Update version in `src/core/common.hpp`:
```cpp
#define COMPRESSOR_VERSION_MAJOR 1
#define COMPRESSOR_VERSION_MINOR 1
#define COMPRESSOR_VERSION_PATCH 0
```

### Build Verification

Before release:
```bash
# Clean build
rm -rf build && mkdir build && cd build

# Release build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run test suite
./compressor benchmark -f ../test/sample-files/mixed.txt

# Memory check
valgrind --leak-check=full ./compressor compress -f ../test/sample-files/low_entropy.txt -a hybrid
```

### Performance Benchmarks

Document performance for release notes:
```bash
# Generate benchmark report
./compressor benchmark -f large_dataset.txt --export-format json --export-file v1.1.0_benchmark.json
```

This development guide provides the foundation for extending and maintaining the Compressor System. Follow these practices to ensure code quality, performance, and maintainability.
