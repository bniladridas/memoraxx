#!/bin/bash

# Build script for memoraxx

set -e  # Exit on error

echo "Building memoraxx..."

# Create build directory if not exists
mkdir -p build

# Change to build directory
cd build

# Run CMake
echo "Running CMake..."
cmake ..

# Build the project
echo "Building..."
cmake --build .

echo "Build completed successfully!"
echo "Run with: ./build/memoraxx"
