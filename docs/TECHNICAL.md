# Compressor System - Technical Documentation

## Architecture Overview

The Compressor System is designed with a modular architecture that separates concerns and provides clear interfaces between components.

### Core Components

#### 1. Algorithm Interface (`core/algorithm.hpp`)
- Abstract base class for all compression algorithms
- Standardized compress/decompress interface
- Performance measurement integration
- Thread-safe design

#### 2. Algorithm Implementations

**Run Length Encoding (RLE)**
- Simple byte-oriented RLE with enhanced mode
- Optimal for low-entropy data with long runs
- Two variants: basic and enhanced with literal sequences
- Entropy-based mode selection

**Huffman Coding**
- Complete canonical Huffman implementation
- Binary tree serialization for self-contained files
- Bit-level packing for optimal compression
- Special handling for single-symbol files

**LZ77 Dictionary Compression**
- Sliding window with hash-based search
- Configurable window and lookahead sizes
- Optimized for repeated patterns and structured data
- Match encoding with distance/length/next-char triplets

**Custom Hybrid Algorithm**
- Adaptive block classification based on entropy and repetition
- Automatic algorithm selection per block
- Preprocessing with byte differencing
- Multi-criteria optimization (size vs. speed)

#### 3. Utility Systems

**File I/O (`utils/file_utils.hpp`)**
- Memory-mapped file reading for large files
- Chunked processing for memory efficiency
- Cross-platform file operations
- Error handling and validation

**CRC32 Checksums (`utils/crc.hpp`)**
- Hardware-optimized CRC32 implementation
- Incremental checksum calculation
- Data integrity verification

**Benchmark Framework (`benchmark/benchmark.hpp`)**
- Multi-algorithm performance comparison
- Statistical analysis and reporting
- Export to multiple formats (CSV, JSON, text)
- Visual progress reporting

#### 4. Command Line Interface (`cli/cli.hpp`)
- Comprehensive argument parsing
- Interactive mode with menu system
- Real-time progress feedback
- Multiple output formats

## Algorithm Details

### Hybrid Algorithm Strategy

The custom hybrid algorithm uses a multi-stage approach:

1. **Preprocessing**: Byte differencing to improve compression ratios
2. **Block Analysis**: Calculate entropy and repetition metrics
3. **Classification**: 
   - Low entropy (< 0.3) → RLE
   - High repetition (> 0.6) → LZ77  
   - Random data → Huffman
   - Mixed → Try all and select best
4. **Postprocessing**: Additional bit packing (future enhancement)

### Performance Optimizations

- **Hash-based LZ77 search**: O(1) average case match finding
- **Canonical Huffman codes**: Optimal bit packing
- **Adaptive block sizing**: Based on input characteristics
- **Memory pooling**: Reduced allocation overhead
- **SIMD potential**: Architecture ready for vectorization

## Building and Installation

### Prerequisites
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16+
- Optional: zlib for comparison benchmarks

### Build Commands
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Development Build
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address"
make -j$(nproc)
```

## Usage Examples

### Basic Compression
```bash
./compressor compress -f input.txt -a hybrid -o output.comp
```

### Benchmark Suite
```bash
./compressor benchmark -f dataset.txt --export-format csv --export-file results.csv
```

### Interactive Mode
```bash
./compressor interactive
```

## Performance Characteristics

### Memory Usage
- **RLE**: O(1) additional memory
- **Huffman**: O(alphabet_size) for tree + O(input_size) for codes
- **LZ77**: O(window_size) for hash tables
- **Hybrid**: O(block_size) for analysis buffers

### Time Complexity
- **RLE**: O(n) linear scan
- **Huffman**: O(n log alphabet_size) for tree construction + O(n) for encoding
- **LZ77**: O(n * window_size) worst case, O(n) average with hashing
- **Hybrid**: O(n) with constant factor overhead for analysis

## Testing Strategy

### Unit Tests
- Algorithm correctness verification
- Roundtrip integrity testing
- Edge case handling (empty files, single bytes, etc.)
- Performance regression detection

### Integration Tests
- Multi-file benchmark suites
- Large file processing (> 1GB)
- Cross-algorithm compatibility
- CLI interface validation

### Benchmark Datasets
- **Low entropy**: Repetitive data (logs, simple images)
- **High entropy**: Random data, encrypted files
- **Structured**: Source code, documents with patterns
- **Mixed**: Real-world data combinations

## Future Enhancements

### Algorithmic Improvements
- **LZMA integration**: For better compression ratios
- **Context modeling**: Predict next symbols based on history
- **Arithmetic coding**: Replace Huffman for better entropy coding
- **Parallel processing**: Block-level parallelization

### System Features
- **Streaming compression**: Process files larger than RAM
- **Network protocols**: Client-server compression
- **Plugin architecture**: Runtime algorithm loading
- **GPU acceleration**: CUDA/OpenCL implementations

### Performance Optimizations
- **SIMD vectorization**: Process multiple bytes simultaneously
- **Cache optimization**: Memory access pattern improvements
- **Lock-free threading**: Reduce synchronization overhead
- **Profile-guided optimization**: Compiler optimization feedback

## Research Applications

This system serves as a platform for:
- **Algorithm comparison studies**: Standardized benchmarking
- **Hybrid technique research**: Multi-algorithm optimization
- **Performance analysis**: Time/space trade-off studies
- **Real-world data analysis**: Compression effectiveness on different data types

The modular design allows easy integration of new algorithms and modifications to existing ones, making it ideal for academic research and algorithm development.
