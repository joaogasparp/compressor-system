# Compressor System – Hybrid and Custom Data Compression Framework

## Overview

**Compressor System** is a command-line based compression framework built in **C++**, designed to **compare, benchmark, and extend traditional data compression techniques**. It supports widely-used compression algorithms and includes a **custom algorithm** developed as a hybrid improvement over existing techniques.

## Features

- **Multi-Algorithm Support**: RLE, LZ77, Huffman, and custom hybrid compression
- **Threaded Execution**: Parallel compression using multithreading
- **Modular Design**: Each algorithm is isolated with common interfaces
- **Interactive CLI Mode**: Optional interactive mode for manual testing
- **Auto-Benchmarking**: Performance comparison across all algorithms
- **Data Integrity**: Built-in checksum validation (CRC32)

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### Basic compression:
```bash
./compressor compress -f input.txt -a huffman -o output.compressed
```

### Decompress:
```bash
./compressor decompress -f output.compressed -o restored.txt
```

### Benchmark all algorithms:
```bash
./compressor benchmark -f input.txt
```

### Interactive mode:
```bash
./compressor interactive
```

## Architecture

- `/src/core` – Data loading, threading infrastructure
- `/src/algorithms` – Independent algorithm implementations
- `/src/benchmark` – Performance measurement tools
- `/src/cli` – Interactive menu system
- `/src/utils` – Utilities (CRC, file I/O, formatting)

## Custom Algorithm

The custom hybrid algorithm combines:
- Run-Length Encoding for low-entropy blocks
- Huffman coding for higher-entropy regions
- Context-based prediction for improved compression ratios

## Performance

The system is optimized for:
- Large file processing with parallel block compression
- Memory-efficient streaming for files larger than RAM
- Real-time performance metrics and progress reporting

## License

This project is developed for educational purposes as part of advanced computer engineering studies.
