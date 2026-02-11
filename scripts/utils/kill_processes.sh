#!/bin/bash
#
# EPH v2.1 - Process Cleanup Script
# Kill all running EPH-related processes
#

echo "EPH v2.1 - Cleaning up running processes..."

# Kill C++ server processes
if pgrep -f "eph_gui_server" > /dev/null; then
    echo "  Killing C++ server (eph_gui_server)..."
    pkill -f "eph_gui_server"
    sleep 0.5
else
    echo "  No C++ server running"
fi

# Kill Python GUI processes
if pgrep -f "python.*main.py" > /dev/null; then
    echo "  Killing Python GUI (main.py)..."
    pkill -f "python.*main.py"
    sleep 0.5
else
    echo "  No Python GUI running"
fi

echo "✓ Cleanup complete"
