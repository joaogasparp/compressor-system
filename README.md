# Compressor System Web Application

A modern web-based file compression tool with a unified C++ server.

## Features

- **Unified Web Server**: Single C++ server serves both API and web interface
- **Multiple Compression Algorithms**: 
  - LZ77 (Lempel-Ziv 77)
  - Huffman Coding
  - RLE (Run-Length Encoding)
- **Drag & Drop File Upload**: Easy file selection with visual feedback
- **Real-time Compression**: Instant compression with progress indicators
- **Compression Metrics**: Detailed statistics including:
  - Original file size
  - Compressed file size
  - Compression ratio
  - Processing time
- **Download Results**: Direct download of compressed files
- **Single Port**: Everything runs on http://localhost:8080

## Building

### Automated Building (Recommended)
```bash
# This handles everything: frontend + backend build
./build-and-run.sh
```

### Manual Building
```bash
# Build backend only
mkdir build
cd build
cmake ..
make
```
## Quick Start

### Option 1: Automated Build and Run (Recommended)

```bash
# Complete setup - builds everything and starts server
./build-and-run.sh
```

### Option 2: Quick Run (if already built)

```bash
# Just start the server
./run.sh
```

### Option 3: Use the Original Startup Script

```bash
./start-application.sh
```

### Option 4: Manual Setup

1. **Build the C++ Server:**
```bash
mkdir -p build
cd build
cmake ..
make web_server
cd ..
```

2. **Start the Server:**
```bash
cd build
./web_server
```

3. **Open in Browser:**
   - Web Application: http://localhost:8080

## Usage

1. **Upload a File**: 
   - Drag and drop a file onto the upload area, or
   - Click to browse and select a file

2. **Select Compression Algorithm**:
   - Choose from LZ77, Huffman, or RLE
   - Each algorithm has different strengths for different file types

3. **Compress**: 
   - Click the "Compress File" button
   - View real-time compression progress

4. **Download Results**:
   - Review compression statistics
   - Download the compressed file

## Algorithm Details

### LZ77 (Lempel-Ziv 77)
- **Best for**: Text files, source code, documents
- **How it works**: Finds repeated sequences and replaces them with references
- **Pros**: Good general-purpose compression, handles repetitive data well

### Huffman Coding
- **Best for**: Files with uneven character distribution
- **How it works**: Assigns shorter codes to more frequent characters
- **Pros**: Optimal for statistical compression, good for text with skewed character frequencies

### RLE (Run-Length Encoding)
- **Best for**: Images with large uniform areas, simple graphics
- **How it works**: Replaces sequences of identical bytes with count + value
- **Pros**: Very fast, excellent for data with many consecutive identical values

## Technical Architecture

### Unified C++ Server
- **HTTP Server**: Custom implementation with REST API
- **Static File Serving**: Serves React build files
- **Compression Engines**: Optimized C++17 implementations
- **CORS Support**: Enables cross-origin requests
- **Multipart Form Handling**: Supports file uploads

### Frontend (React)
- **Modern React**: Built and bundled for production
- **Material-UI**: Professional, responsive design
- **File Upload**: Drag-and-drop functionality
- **API Integration**: Direct communication with C++ server

## API Endpoints

### POST /compress
Compress a file using the specified algorithm.

**Request:**
- Method: POST
- Content-Type: multipart/form-data
- Body:
  - `file`: File to compress
  - `algorithm`: Compression algorithm (`lz77`, `huffman`, or `rle`)

**Response:**
```json
{
    "success": true,
    "original_size": 1024,
    "compressed_size": 512,
    "compression_ratio": 0.5,
    "compression_time_ms": 15,
    "algorithm": "lz77",
    "verified": true,
    "compressed_data": "base64_encoded_data"
}
```

### GET /algorithms
Get list of available compression algorithms.

**Response:**
```json
{
    "algorithms": ["lz77", "huffman", "rle"]
}
```

## File Structure

```
compressor-system/
├── src/                    # C++ source code
│   ├── algorithms/         # Compression algorithms (LZ77, Huffman, RLE)
│   ├── core/              # Core compression engine interfaces
│   ├── utils/             # Utility functions (CRC, file operations)
│   └── web_server.cpp     # Unified HTTP server
├── web-app/               # React frontend (production build)
│   └── build/             # Compiled React application
├── build/                 # C++ build output
│   └── web_server         # Executable
├── CMakeLists.txt         # Build configuration
├── start-application.sh   # Startup script
└── README.md             # This file
```

## Development

### Prerequisites
- C++17 compatible compiler (GCC 7+ or Clang 5+)
- CMake 3.16+

### Building
```bash
mkdir -p build
cd build
cmake ..
make web_server
```

### Running
```bash
./build/web_server
# Open: http://localhost:8080
```

## License

This project is open source and available under the MIT License.
