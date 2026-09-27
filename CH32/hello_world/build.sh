#!/bin/bash
# Build script for CH32 Hello World project

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Create build directory
echo -e "${BLUE}Creating build directory...${NC}"
mkdir -p build
cd build

# Run CMake
echo -e "${BLUE}Running CMake...${NC}"
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build project
echo -e "${BLUE}Building project...${NC}"
make -j$(nproc)

# Summary
echo -e "${GREEN}"
echo "Build complete!"
echo "Output files:"
echo "  ELF:  CH32_HelloWorld.elf"
echo "  HEX:  CH32_HelloWorld.hex"
echo "  BIN:  CH32_HelloWorld.bin"
echo -e "${NC}"
