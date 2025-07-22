#!/bin/bash

# Build script for Compressor System

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building Compressor System...${NC}"

# Create build directory
if [ ! -d "build" ]; then
    mkdir build
    echo -e "${YELLOW}Created build directory${NC}"
fi

cd build

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo -e "${RED}CMake configuration failed!${NC}"
    exit 1
fi

# Build
echo -e "${YELLOW}Building...${NC}"
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "Executable location: ${YELLOW}build/compressor${NC}"

# Run basic test
echo -e "${YELLOW}Running basic test...${NC}"
echo "Hello, World! This is a test file for the compressor system." > test_input.txt

./compressor compress -f test_input.txt -a huffman -o test_compressed.bin
if [ $? -eq 0 ]; then
    ./compressor decompress -f test_compressed.bin -o test_output.txt
    if [ $? -eq 0 ]; then
        if cmp -s test_input.txt test_output.txt; then
            echo -e "${GREEN}Basic compression/decompression test passed!${NC}"
            rm -f test_input.txt test_compressed.bin test_output.txt
        else
            echo -e "${RED}Files don't match after compression/decompression!${NC}"
        fi
    else
        echo -e "${RED}Decompression test failed!${NC}"
    fi
else
    echo -e "${RED}Compression test failed!${NC}"
fi

echo -e "${GREEN}Setup complete! Use './build/compressor help' for usage information.${NC}"
