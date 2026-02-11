#!/bin/bash

echo "EPH v2.1 - GUI Server Build Script"
echo "==================================="
echo ""

# Navigate to project root (script is in scripts/)
cd "$(dirname "$0")/.." || exit 1

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

cd build || exit 1

# Run CMake configuration
echo "Configuring with CMake..."
cmake .. -DBUILD_GUI_SERVER=ON

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: CMake configuration failed"
    exit 1
fi

echo ""
echo "Building eph_gui_server..."
cmake --build . --target eph_gui_server

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Build failed"
    exit 1
fi

echo ""
echo "✅ Build successful!"
echo ""
echo "Executable location: ./build/cpp_server/eph_gui_server"
echo ""
echo "To run the integration test:"
echo "  ./scripts/test_udp_communication.sh"
echo ""
