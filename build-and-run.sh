#!/bin/bash

# File Compression System - Build and Run Script
# This script builds both frontend and backend, then starts the server

set -e  # Exit on any error

echo "=== File Compression System Setup ==="
echo

# Get the script directory (project root)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "Building Frontend (React Application)..."
echo "----------------------------------------"
cd web-app

# Check if node_modules exists, if not run npm install
if [ ! -d "node_modules" ]; then
    echo "Installing npm dependencies..."
    npm install
else
    echo "Dependencies already installed, skipping npm install"
fi

echo "Building React production build..."
npm run build

echo "Frontend build completed"
echo

cd ..

echo "Building Backend (C++ Server)..."
echo "-----------------------------------"

# Create build directory if it doesn't exist
mkdir -p build
cd build

echo "Running CMake configuration..."
cmake ..

echo "Compiling C++ server..."
make -j$(nproc)

echo "Backend build completed"
echo

cd ..

echo "Starting Web Server..."
echo "-------------------------"
echo "Server will start on: http://localhost:8080"
echo "Press Ctrl+C to stop the server"
echo

# Run the server
./build/web_server
