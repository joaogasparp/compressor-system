#!/bin/bash

# Quick Start Script - Run the compression system
# Use this if you've already built the project with build-and-run.sh

set -e

echo "Starting File Compression System..."
echo "Server will be available at: http://localhost:8080"
echo "Press Ctrl+C to stop the server"
echo

# Check if the web_server binary exists
if [ ! -f "./build/web_server" ]; then
    echo "Error: web_server binary not found!"
    echo "Please run ./build-and-run.sh first to build the project."
    exit 1
fi

# Check if the React build exists
if [ ! -d "./web-app/build" ]; then
    echo "Error: React build not found!"
    echo "Please run ./build-and-run.sh first to build the project."
    exit 1
fi

# Start the server
./build/web_server
