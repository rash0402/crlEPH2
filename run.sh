#!/bin/bash
#
# EPH v2.1 - Automated Experiment Start Script
# Clean process management and automatic server/GUI startup
#

set -e  # Exit on error

# Navigate to project root
cd "$(dirname "$0")" || exit 1

echo "EPH v2.1 - Automated Experiment Launcher"
echo "========================================="
echo ""

# Step 1: Clean up any existing processes
echo "[1/4] Cleaning up existing processes..."
./scripts/utils/kill_processes.sh
echo ""

# Step 2: Build C++ server if needed
echo "[2/4] Checking C++ server build..."
if [ ! -f "build/src/cpp_server/eph_gui_server" ]; then
    echo "  Server not found. Building..."
    ./scripts/build/build_gui_server.sh
else
    echo "  ✓ Server binary exists"
fi
echo ""

# Step 3: Start C++ server in background
echo "[3/4] Starting C++ server..."
LOG_FILE="logs/server/eph_server_$(date +%Y-%m-%d_%H%M%S).log"
./build/src/cpp_server/eph_gui_server > "$LOG_FILE" 2>&1 &
SERVER_PID=$!
echo "  ✓ C++ server started (PID: $SERVER_PID)"
echo "    Log: $LOG_FILE"
ln -sf "$(basename "$LOG_FILE")" logs/server/eph_server_latest.log
echo ""

# Wait for server to initialize
sleep 1

# Step 4: Start Python GUI
echo "[4/4] Starting Python GUI..."
echo "  Log: GUI output will appear below"
echo "  Press Ctrl+C to stop both processes"
echo ""
echo "----------------------------------------"

# Trap Ctrl+C to cleanly shut down both processes
cleanup() {
    echo ""
    echo "----------------------------------------"
    echo "Shutting down..."
    if [ -n "$GUI_PID" ]; then
        echo "  Stopping Python GUI (PID: $GUI_PID)..."
        kill $GUI_PID 2>/dev/null || true
    fi
    if [ -n "$SERVER_PID" ]; then
        echo "  Stopping C++ server (PID: $SERVER_PID)..."
        kill $SERVER_PID 2>/dev/null || true
    fi
    echo "✓ Experiment stopped"
    exit 0
}

trap cleanup INT TERM

# Start GUI in foreground
cd src/gui || exit 1
python3 main.py &
GUI_PID=$!

# Wait for GUI process
wait $GUI_PID

# If GUI exits normally, clean up server
cleanup
