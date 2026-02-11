#!/bin/bash

echo "EPH v2.1 - UDP Communication Test"
echo "==================================="
echo ""

# Check if C++ server is built
if [ ! -f "build/cpp_server/eph_gui_server" ]; then
    echo "ERROR: C++ server not built"
    echo "Run: cd build && cmake --build . --target eph_gui_server"
    exit 1
fi

# Check if Python env is ready
if [ ! -d "gui/.venv" ]; then
    echo "ERROR: Python venv not found"
    echo "Run: cd gui && python3 -m venv .venv && pip install -r requirements.txt"
    exit 1
fi

echo "Starting C++ server in background..."
./build/cpp_server/eph_gui_server > /tmp/eph_server.log 2>&1 &
SERVER_PID=$!
echo "  PID: $SERVER_PID"
sleep 2

echo ""
echo "Starting Python GUI..."
cd gui
source .venv/bin/activate
python main.py &
GUI_PID=$!
cd ..
echo "  PID: $GUI_PID"

echo ""
echo "Test running..."
echo "  - C++ server sending state data on port 5555"
echo "  - Python GUI receiving and displaying"
echo "  - Check GUI window for 'Connected' status and incrementing steps"
echo ""
echo "Press Ctrl+C to stop both processes"

# Wait for user interrupt
trap "kill $SERVER_PID $GUI_PID 2>/dev/null; exit" INT
wait
