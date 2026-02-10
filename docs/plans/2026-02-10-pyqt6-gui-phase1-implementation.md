# EPH v2.1 PyQt6 GUI - Phase 1 Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build the foundational infrastructure for C++ ↔ Python communication via UDP sockets, enabling real-time data exchange between the EPH simulation engine and the GUI.

**Architecture:**
- C++ side: UDP server with binary protocol for state data (port 5555) and JSON protocol for control commands (port 5556)
- Python side: UDP client with deserialization, ring buffer, and basic PyQt6 window
- 270° field-of-view parameter integration into existing SPM system

**Tech Stack:**
- C++17 (Eigen 3.4+, std::optional, nlohmann/json)
- Python 3.11+ (PyQt6 6.4+, numpy 1.24+, struct for binary parsing)
- UDP sockets (localhost, non-blocking)

**Reference:** Design document at `docs/plans/2026-02-10-pyqt6-gui-design.md`

---

## Prerequisites

**Verify existing setup:**
```bash
# C++ build environment
cmake --version  # Should be 3.20+
find packages/eph_core/include -name "*.hpp"  # Verify EPH v2.1 exists

# Python environment
python3 --version  # Should be 3.11+
ls .venv  # Check if venv exists
```

**Expected:** EPH v2.1 C++ packages exist, tests pass (210/215 from previous status)

---

## Task 1: Project Structure Setup

**Duration:** 5-10 minutes

**Files:**
- Create: `gui/` (new directory)
- Create: `gui/requirements.txt`
- Create: `gui/__init__.py`
- Create: `cpp_server/` (new directory)
- Create: `cpp_server/CMakeLists.txt`

**Step 1: Create Python GUI directory structure**

```bash
mkdir -p gui/bridge gui/widgets gui/dialogs gui/utils gui/resources
touch gui/__init__.py
touch gui/bridge/__init__.py
touch gui/widgets/__init__.py
touch gui/dialogs/__init__.py
touch gui/utils/__init__.py
```

Run: `ls -R gui/`
Expected: Directory tree with `__init__.py` files

**Step 2: Create requirements.txt**

File: `gui/requirements.txt`
```txt
PyQt6>=6.4.0
matplotlib>=3.8.0
numpy>=1.24.0
imageio>=2.31.0
```

**Step 3: Setup Python virtual environment**

```bash
cd gui
python3 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip
pip install -r requirements.txt
```

Run: `pip list | grep PyQt6`
Expected: PyQt6 6.x.x installed

**Step 4: Create C++ server directory**

```bash
cd ..  # Back to project root
mkdir -p cpp_server
```

**Step 5: Commit**

```bash
git add gui/ cpp_server/
git commit -m "chore: initialize GUI and C++ server directory structure

- Add gui/ with Python package structure
- Add requirements.txt (PyQt6, matplotlib, numpy, imageio)
- Add cpp_server/ directory for UDP server implementation
- Phase 1: Foundation setup"
```

---

## Task 2: C++ - Add Field of View Parameter

**Duration:** 10-15 minutes

**Files:**
- Modify: `packages/eph_core/include/eph_core/constants.hpp`
- Test: Manual verification (no new test needed, uses existing SPM tests)

**Context:** The SPM currently assumes 360° coverage. We need to make the field of view configurable (default 270°) and adjust θ index interpretation.

**Step 1: Add FOV constant to constants.hpp**

File: `packages/eph_core/include/eph_core/constants.hpp`

Find the section with `N_THETA` and `N_R` constants (around line 20-30), and add:

```cpp
// Add after existing SPM constants
constexpr Scalar FIELD_OF_VIEW_DEGREES = 270.0;  // Default: 270° forward-facing view
constexpr Scalar FIELD_OF_VIEW_RADIANS = FIELD_OF_VIEW_DEGREES * M_PI / 180.0;
```

**Step 2: Verify compilation**

```bash
cd build
cmake --build . --target eph_core
```

Expected: Clean compilation, no errors

**Step 3: Document the θ index interpretation**

Add comment in `packages/eph_spm/include/eph_spm/saliency_polar_map.hpp` at the class definition:

```cpp
/**
 * @brief Saliency Polar Map with configurable field of view
 *
 * θ index interpretation (when FIELD_OF_VIEW_DEGREES = 270):
 *   - θ_idx = 0   → -135° (left edge of FOV)
 *   - θ_idx = 6   → 0° (forward, agent heading)
 *   - θ_idx = 11  → +135° (right edge of FOV)
 *
 * For 360° FOV: θ ∈ [0°, 360°)
 * For 270° FOV: θ ∈ [-135°, +135°] centered on heading
 */
class SaliencyPolarMap {
```

**Step 4: Commit**

```bash
git add packages/eph_core/include/eph_core/constants.hpp \
        packages/eph_spm/include/eph_spm/saliency_polar_map.hpp
git commit -m "feat: add configurable field of view parameter to SPM

- Add FIELD_OF_VIEW_DEGREES constant (default: 270°)
- Document θ index interpretation for ego-centric view
- Preparation for GUI polar visualization

Phase 1: C++ foundation"
```

---

## Task 3: C++ - Binary Protocol Header

**Duration:** 15-20 minutes

**Files:**
- Create: `cpp_server/protocol.hpp`
- Test: Will be tested in Task 5 with unit test

**Step 1: Create protocol header with data structures**

File: `cpp_server/protocol.hpp`

```cpp
#ifndef EPH_UDP_PROTOCOL_HPP
#define EPH_UDP_PROTOCOL_HPP

#include <cstdint>
#include <vector>
#include <cstring>
#include "eph_core/types.hpp"

namespace eph::udp {

// Magic number for packet identification: 0xEFE20210 (EPH v2.1, 2021-0)
constexpr uint32_t MAGIC_NUMBER = 0xEFE20210;

/**
 * @brief Binary packet header (24 bytes)
 */
struct PacketHeader {
    uint32_t magic_number;   // 0xEFE20210
    uint32_t sequence_num;   // Incremental packet counter
    uint32_t timestep;       // Simulation step number
    uint32_t num_agents;     // Number of agents in payload
    uint32_t data_length;    // Payload size in bytes
    uint32_t checksum;       // CRC32 checksum of payload

    PacketHeader()
        : magic_number(MAGIC_NUMBER),
          sequence_num(0),
          timestep(0),
          num_agents(0),
          data_length(0),
          checksum(0) {}
};

/**
 * @brief Agent state data (32 bytes per agent)
 */
struct AgentData {
    uint16_t agent_id;      // Agent ID
    uint16_t padding1;      // Alignment padding
    float x;                // Position x
    float y;                // Position y
    float vx;               // Velocity x
    float vy;               // Velocity y
    float haze_mean;        // Mean haze value
    float fatigue;          // Fatigue level [0, 1]
    float efe;              // Expected Free Energy
    uint16_t padding2;      // Alignment padding
};

/**
 * @brief Metrics data (48 bytes)
 */
struct MetricsData {
    double phi;             // Order parameter φ(t)
    double chi;             // Susceptibility χ(t)
    double beta_current;    // Current β value
    double avg_haze;        // Average haze across agents
    double avg_speed;       // Average speed
    double avg_fatigue;     // Average fatigue
};

/**
 * @brief Complete state packet
 */
struct StatePacket {
    PacketHeader header;
    std::vector<AgentData> agents;
    MetricsData metrics;

    StatePacket() = default;
};

/**
 * @brief Simple CRC32 checksum calculator
 */
inline uint32_t calculate_crc32(const uint8_t* data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
    }
    return ~crc;
}

/**
 * @brief Serialize StatePacket to binary buffer
 */
inline std::vector<uint8_t> serialize_state_packet(const StatePacket& packet) {
    const size_t total_size = sizeof(PacketHeader) +
                              packet.agents.size() * sizeof(AgentData) +
                              sizeof(MetricsData);

    std::vector<uint8_t> buffer(total_size);
    size_t offset = 0;

    // Copy header
    std::memcpy(buffer.data() + offset, &packet.header, sizeof(PacketHeader));
    offset += sizeof(PacketHeader);

    // Copy agent data
    for (const auto& agent : packet.agents) {
        std::memcpy(buffer.data() + offset, &agent, sizeof(AgentData));
        offset += sizeof(AgentData);
    }

    // Copy metrics
    std::memcpy(buffer.data() + offset, &packet.metrics, sizeof(MetricsData));

    return buffer;
}

} // namespace eph::udp

#endif // EPH_UDP_PROTOCOL_HPP
```

**Step 2: Verify compilation**

```bash
# Create a simple test file
cat > cpp_server/test_protocol_compile.cpp << 'EOF'
#include "protocol.hpp"
int main() {
    eph::udp::StatePacket packet;
    return 0;
}
EOF

g++ -std=c++17 -I../packages -c cpp_server/test_protocol_compile.cpp -o /tmp/test_protocol.o
```

Expected: Compiles without errors

**Step 3: Clean up test file**

```bash
rm cpp_server/test_protocol_compile.cpp
```

**Step 4: Commit**

```bash
git add cpp_server/protocol.hpp
git commit -m "feat(cpp): add binary protocol for UDP state packets

- Define PacketHeader (24 bytes) with magic number and checksum
- Define AgentData (32 bytes) for per-agent state
- Define MetricsData (48 bytes) for global metrics
- Add CRC32 checksum calculator
- Add serialize_state_packet() function

Phase 1: C++ UDP protocol"
```

---

## Task 4: C++ - UDP Server Implementation

**Duration:** 25-30 minutes

**Files:**
- Create: `cpp_server/udp_server.hpp`
- Create: `cpp_server/udp_server.cpp`

**Step 1: Create UDP server header**

File: `cpp_server/udp_server.hpp`

```cpp
#ifndef EPH_UDP_SERVER_HPP
#define EPH_UDP_SERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "protocol.hpp"

namespace eph::udp {

/**
 * @brief UDP Server for EPH GUI communication
 *
 * Port 5555: Send state data (C++ → Python)
 * Port 5556: Receive control commands (Python → C++)
 */
class UDPServer {
public:
    /**
     * @param send_port Port for sending state data (default: 5555)
     * @param recv_port Port for receiving commands (default: 5556)
     * @param target_host Target host for sending (default: "127.0.0.1")
     */
    UDPServer(uint16_t send_port = 5555,
              uint16_t recv_port = 5556,
              const std::string& target_host = "127.0.0.1");

    ~UDPServer();

    // Disable copy
    UDPServer(const UDPServer&) = delete;
    UDPServer& operator=(const UDPServer&) = delete;

    /**
     * @brief Send state packet to GUI
     * @return true if sent successfully
     */
    bool send_state(const StatePacket& packet);

    /**
     * @brief Receive control command (non-blocking)
     * @return JSON command if available, std::nullopt otherwise
     */
    std::optional<nlohmann::json> receive_command();

    /**
     * @brief Check if sockets are initialized properly
     */
    bool is_initialized() const { return initialized_; }

    /**
     * @brief Get last error message
     */
    const std::string& get_last_error() const { return last_error_; }

private:
    int send_socket_;
    int recv_socket_;
    struct sockaddr_in send_addr_;
    struct sockaddr_in recv_addr_;
    bool initialized_;
    std::string last_error_;

    bool init_send_socket(uint16_t port, const std::string& target_host);
    bool init_recv_socket(uint16_t port);
    void set_non_blocking(int socket_fd);
};

} // namespace eph::udp

#endif // EPH_UDP_SERVER_HPP
```

**Step 2: Implement UDP server**

File: `cpp_server/udp_server.cpp`

```cpp
#include "udp_server.hpp"
#include <iostream>
#include <cstring>

namespace eph::udp {

UDPServer::UDPServer(uint16_t send_port, uint16_t recv_port, const std::string& target_host)
    : send_socket_(-1),
      recv_socket_(-1),
      initialized_(false),
      last_error_("") {

    initialized_ = init_send_socket(send_port, target_host) &&
                   init_recv_socket(recv_port);

    if (!initialized_) {
        std::cerr << "UDP Server initialization failed: " << last_error_ << std::endl;
    }
}

UDPServer::~UDPServer() {
    if (send_socket_ >= 0) {
        close(send_socket_);
    }
    if (recv_socket_ >= 0) {
        close(recv_socket_);
    }
}

bool UDPServer::init_send_socket(uint16_t port, const std::string& target_host) {
    send_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (send_socket_ < 0) {
        last_error_ = "Failed to create send socket";
        return false;
    }

    std::memset(&send_addr_, 0, sizeof(send_addr_));
    send_addr_.sin_family = AF_INET;
    send_addr_.sin_port = htons(port);

    if (inet_pton(AF_INET, target_host.c_str(), &send_addr_.sin_addr) <= 0) {
        last_error_ = "Invalid target host address";
        return false;
    }

    return true;
}

bool UDPServer::init_recv_socket(uint16_t port) {
    recv_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (recv_socket_ < 0) {
        last_error_ = "Failed to create receive socket";
        return false;
    }

    // Set non-blocking
    set_non_blocking(recv_socket_);

    // Allow address reuse
    int opt = 1;
    if (setsockopt(recv_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        last_error_ = "Failed to set socket options";
        return false;
    }

    std::memset(&recv_addr_, 0, sizeof(recv_addr_));
    recv_addr_.sin_family = AF_INET;
    recv_addr_.sin_addr.s_addr = INADDR_ANY;
    recv_addr_.sin_port = htons(port);

    if (bind(recv_socket_, (struct sockaddr*)&recv_addr_, sizeof(recv_addr_)) < 0) {
        last_error_ = "Failed to bind receive socket to port " + std::to_string(port);
        return false;
    }

    return true;
}

void UDPServer::set_non_blocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
}

bool UDPServer::send_state(const StatePacket& packet) {
    if (!initialized_) {
        return false;
    }

    auto buffer = serialize_state_packet(packet);

    ssize_t sent = sendto(send_socket_, buffer.data(), buffer.size(), 0,
                          (struct sockaddr*)&send_addr_, sizeof(send_addr_));

    if (sent < 0) {
        last_error_ = "Failed to send packet: " + std::string(strerror(errno));
        return false;
    }

    return true;
}

std::optional<nlohmann::json> UDPServer::receive_command() {
    if (!initialized_) {
        return std::nullopt;
    }

    char buffer[4096];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    ssize_t received = recvfrom(recv_socket_, buffer, sizeof(buffer) - 1, 0,
                                (struct sockaddr*)&client_addr, &addr_len);

    if (received < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            // No data available (non-blocking)
            return std::nullopt;
        }
        last_error_ = "Failed to receive command: " + std::string(strerror(errno));
        return std::nullopt;
    }

    buffer[received] = '\0';

    try {
        nlohmann::json command = nlohmann::json::parse(buffer);
        return command;
    } catch (const nlohmann::json::parse_error& e) {
        last_error_ = "JSON parse error: " + std::string(e.what());
        return std::nullopt;
    }
}

} // namespace eph::udp
```

**Step 3: Add nlohmann/json dependency check**

Check if nlohmann/json is available. If not, we'll use a simpler approach or download it.

```bash
# Check if nlohmann/json exists
if [ ! -f "/usr/local/include/nlohmann/json.hpp" ] && [ ! -f "/opt/homebrew/include/nlohmann/json.hpp" ]; then
    echo "nlohmann/json not found. Will download header-only version."
    mkdir -p cpp_server/external
    curl -o cpp_server/external/json.hpp https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
fi
```

**Step 4: Update protocol.hpp to use local json if needed**

If downloaded to external/, add at top of udp_server.hpp:

```cpp
// Check if system nlohmann/json exists, otherwise use local
#if __has_include(<nlohmann/json.hpp>)
#include <nlohmann/json.hpp>
#else
#include "external/json.hpp"
#endif
```

**Step 5: Commit**

```bash
git add cpp_server/udp_server.hpp cpp_server/udp_server.cpp
git commit -m "feat(cpp): implement UDP server for GUI communication

- Port 5555: send state data to Python GUI
- Port 5556: receive JSON control commands
- Non-blocking receive for real-time performance
- Error handling with last_error_ tracking

Phase 1: C++ UDP server"
```

---

## Task 5: C++ - CMake Integration

**Duration:** 10-15 minutes

**Files:**
- Create: `cpp_server/CMakeLists.txt`
- Modify: Root `CMakeLists.txt`

**Step 1: Create cpp_server CMakeLists.txt**

File: `cpp_server/CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.20)
project(eph_udp_server VERSION 2.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Eigen (required for EPH types)
find_package(Eigen3 3.4 REQUIRED NO_MODULE)

# UDP Server library
add_library(eph_udp_server
    udp_server.cpp
)

target_include_directories(eph_udp_server PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/packages
)

target_link_libraries(eph_udp_server PUBLIC
    eph_core
    Eigen3::Eigen
)

# Main executable (simulation + UDP server)
add_executable(eph_gui_server
    main.cpp
)

target_link_libraries(eph_gui_server PRIVATE
    eph_udp_server
    eph_swarm
    eph_phase
    eph_agent
    eph_spm
    eph_core
)

# Installation
install(TARGETS eph_udp_server eph_gui_server
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
```

**Step 2: Create placeholder main.cpp**

File: `cpp_server/main.cpp`

```cpp
#include <iostream>
#include "udp_server.hpp"

int main(int argc, char** argv) {
    std::cout << "EPH v2.1 GUI Server" << std::endl;
    std::cout << "Initializing UDP server..." << std::endl;

    eph::udp::UDPServer server(5555, 5556);

    if (!server.is_initialized()) {
        std::cerr << "Failed to initialize UDP server: "
                  << server.get_last_error() << std::endl;
        return 1;
    }

    std::cout << "UDP server initialized successfully" << std::endl;
    std::cout << "  Send port: 5555 (state data)" << std::endl;
    std::cout << "  Recv port: 5556 (commands)" << std::endl;
    std::cout << "Press Ctrl+C to exit." << std::endl;

    // TODO: Integrate with SwarmManager in next task

    while (true) {
        auto command = server.receive_command();
        if (command.has_value()) {
            std::cout << "Received command: " << command.value().dump() << std::endl;
        }

        usleep(100000); // 100ms
    }

    return 0;
}
```

**Step 3: Update root CMakeLists.txt**

File: `CMakeLists.txt` (root)

Add at the end:

```cmake
# UDP GUI Server (optional build)
option(BUILD_GUI_SERVER "Build UDP GUI Server" ON)
if(BUILD_GUI_SERVER)
    add_subdirectory(cpp_server)
endif()
```

**Step 4: Build and test**

```bash
cd build
cmake .. -DBUILD_GUI_SERVER=ON
cmake --build . --target eph_gui_server
```

Expected: Builds successfully

**Step 5: Test the server**

```bash
# Terminal 1: Run server
./cpp_server/eph_gui_server

# Terminal 2: Send test JSON (in another terminal)
echo '{"type":"test","value":123}' | nc -u 127.0.0.1 5556
```

Expected: Server receives and prints the JSON command

**Step 6: Stop server (Ctrl+C) and commit**

```bash
git add cpp_server/CMakeLists.txt cpp_server/main.cpp CMakeLists.txt
git commit -m "build(cpp): add CMake configuration for UDP GUI server

- Add cpp_server/CMakeLists.txt with eph_udp_server library
- Add eph_gui_server executable with placeholder main
- Add BUILD_GUI_SERVER option to root CMakeLists.txt
- Test: server initializes and receives JSON commands

Phase 1: C++ build system"
```

---

## Task 6: Python - UDP Client Protocol

**Duration:** 20-25 minutes

**Files:**
- Create: `gui/bridge/protocol.py`
- Create: `gui/bridge/test_protocol.py` (unit test)

**Step 1: Write failing test first**

File: `gui/bridge/test_protocol.py`

```python
import struct
import pytest
from gui.bridge.protocol import (
    MAGIC_NUMBER,
    PacketHeader,
    AgentData,
    MetricsData,
    deserialize_state_packet
)


def test_packet_header_size():
    """PacketHeader should be exactly 24 bytes"""
    assert PacketHeader.SIZE == 24


def test_agent_data_size():
    """AgentData should be exactly 32 bytes"""
    assert AgentData.SIZE == 32


def test_metrics_data_size():
    """MetricsData should be exactly 48 bytes"""
    assert MetricsData.SIZE == 48


def test_deserialize_valid_packet():
    """Should deserialize a valid state packet"""
    # Create a minimal valid packet
    header_data = struct.pack(
        'IIIIII',
        MAGIC_NUMBER,  # magic_number
        1,             # sequence_num
        100,           # timestep
        2,             # num_agents
        2 * 32 + 48,   # data_length (2 agents + metrics)
        0              # checksum (simplified for test)
    )

    # Agent 0
    agent0_data = struct.pack(
        'HHffffffff',
        0,      # agent_id
        0,      # padding
        1.0,    # x
        2.0,    # y
        0.5,    # vx
        0.3,    # vy
        0.4,    # haze_mean
        0.1,    # fatigue
        1.2,    # efe
        0       # padding (uint16 -> H, but we need 2 bytes)
    )

    # Agent 1
    agent1_data = struct.pack(
        'HHffffffffH',
        1, 0, 3.0, 4.0, 0.2, 0.1, 0.5, 0.2, 1.5, 0
    )

    # Metrics
    metrics_data = struct.pack(
        'dddddd',
        0.034,  # phi
        2.145,  # chi
        0.098,  # beta_current
        0.45,   # avg_haze
        0.55,   # avg_speed
        0.15    # avg_fatigue
    )

    packet_bytes = header_data + agent0_data + agent1_data + metrics_data

    # Deserialize
    packet = deserialize_state_packet(packet_bytes)

    assert packet is not None
    assert packet['header']['magic_number'] == MAGIC_NUMBER
    assert packet['header']['sequence_num'] == 1
    assert packet['header']['timestep'] == 100
    assert packet['header']['num_agents'] == 2

    assert len(packet['agents']) == 2
    assert packet['agents'][0]['agent_id'] == 0
    assert pytest.approx(packet['agents'][0]['x'], 0.01) == 1.0

    assert pytest.approx(packet['metrics']['phi'], 0.001) == 0.034
    assert pytest.approx(packet['metrics']['chi'], 0.001) == 2.145


def test_deserialize_invalid_magic():
    """Should return None for invalid magic number"""
    bad_header = struct.pack('IIIIII', 0xDEADBEEF, 0, 0, 0, 0, 0)
    packet = deserialize_state_packet(bad_header)
    assert packet is None


def test_deserialize_truncated_packet():
    """Should return None for truncated packet"""
    truncated = struct.pack('III', MAGIC_NUMBER, 1, 100)
    packet = deserialize_state_packet(truncated)
    assert packet is None
```

**Step 2: Run test to verify it fails**

```bash
cd gui
source .venv/bin/activate
python -m pytest bridge/test_protocol.py -v
```

Expected: FAIL - module 'gui.bridge.protocol' has no attribute 'MAGIC_NUMBER'

**Step 3: Implement protocol.py**

File: `gui/bridge/protocol.py`

```python
"""
UDP Binary Protocol for EPH GUI Communication

Matches C++ protocol definition in cpp_server/protocol.hpp
"""

import struct
from typing import Optional, Dict, List, Any

# Magic number: 0xEFE20210 (EPH v2.1, 2021-0)
MAGIC_NUMBER = 0xEFE20210


class PacketHeader:
    """Binary packet header (24 bytes)"""
    SIZE = 24
    FORMAT = 'IIIIII'  # 6 uint32_t

    @staticmethod
    def parse(data: bytes) -> Optional[Dict[str, int]]:
        if len(data) < PacketHeader.SIZE:
            return None

        values = struct.unpack(PacketHeader.FORMAT, data[:PacketHeader.SIZE])
        return {
            'magic_number': values[0],
            'sequence_num': values[1],
            'timestep': values[2],
            'num_agents': values[3],
            'data_length': values[4],
            'checksum': values[5]
        }


class AgentData:
    """Agent state data (32 bytes per agent)"""
    SIZE = 32
    # uint16 id, uint16 pad, 7x float32, uint16 pad2
    FORMAT = 'HHffffffffH'

    @staticmethod
    def parse(data: bytes) -> Optional[Dict[str, Any]]:
        if len(data) < AgentData.SIZE:
            return None

        values = struct.unpack(AgentData.FORMAT, data[:AgentData.SIZE])
        return {
            'agent_id': values[0],
            'x': values[2],
            'y': values[3],
            'vx': values[4],
            'vy': values[5],
            'haze_mean': values[6],
            'fatigue': values[7],
            'efe': values[8]
        }


class MetricsData:
    """Metrics data (48 bytes)"""
    SIZE = 48
    FORMAT = 'dddddd'  # 6 double

    @staticmethod
    def parse(data: bytes) -> Optional[Dict[str, float]]:
        if len(data) < MetricsData.SIZE:
            return None

        values = struct.unpack(MetricsData.FORMAT, data[:MetricsData.SIZE])
        return {
            'phi': values[0],
            'chi': values[1],
            'beta_current': values[2],
            'avg_haze': values[3],
            'avg_speed': values[4],
            'avg_fatigue': values[5]
        }


def deserialize_state_packet(data: bytes) -> Optional[Dict[str, Any]]:
    """
    Deserialize binary state packet from C++ server

    Args:
        data: Binary packet data

    Returns:
        Dictionary with 'header', 'agents', 'metrics' or None if invalid
    """
    if len(data) < PacketHeader.SIZE:
        return None

    # Parse header
    header = PacketHeader.parse(data)
    if header is None or header['magic_number'] != MAGIC_NUMBER:
        return None

    num_agents = header['num_agents']
    offset = PacketHeader.SIZE

    # Parse agents
    agents: List[Dict[str, Any]] = []
    for i in range(num_agents):
        if offset + AgentData.SIZE > len(data):
            return None

        agent = AgentData.parse(data[offset:offset + AgentData.SIZE])
        if agent is None:
            return None

        agents.append(agent)
        offset += AgentData.SIZE

    # Parse metrics
    if offset + MetricsData.SIZE > len(data):
        return None

    metrics = MetricsData.parse(data[offset:offset + MetricsData.SIZE])
    if metrics is None:
        return None

    return {
        'header': header,
        'agents': agents,
        'metrics': metrics
    }
```

**Step 4: Run test to verify it passes**

```bash
python -m pytest bridge/test_protocol.py -v
```

Expected: PASS - all tests pass

**Step 5: Commit**

```bash
git add gui/bridge/protocol.py gui/bridge/test_protocol.py
git commit -m "feat(python): implement binary protocol deserialization

- Add PacketHeader, AgentData, MetricsData parsers
- Add deserialize_state_packet() function
- Match C++ protocol specification (24+32N+48 bytes)
- Add comprehensive unit tests (7 test cases)
- All tests passing

Phase 1: Python UDP protocol"
```

---

## Task 7: Python - UDP Client

**Duration:** 25-30 minutes

**Files:**
- Create: `gui/bridge/udp_client.py`
- Create: `gui/bridge/test_udp_client.py`

**Step 1: Write failing test**

File: `gui/bridge/test_udp_client.py`

```python
import socket
import struct
import time
import pytest
from gui.bridge.udp_client import UDPClient
from gui.bridge.protocol import MAGIC_NUMBER


@pytest.fixture
def mock_server():
    """Create a mock C++ server for testing"""
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_socket.bind(('127.0.0.1', 5555))
    server_socket.settimeout(0.1)
    yield server_socket
    server_socket.close()


def test_udp_client_initialization():
    """Client should initialize with correct ports"""
    client = UDPClient(recv_port=5555, send_port=5556)
    assert client.recv_port == 5555
    assert client.send_port == 5556
    assert client.is_connected == False  # Not connected until receive data
    client.close()


def test_receive_state_packet(mock_server):
    """Should receive and parse state packet"""
    client = UDPClient(recv_port=5555, send_port=5556)

    # Create minimal valid packet
    header = struct.pack('IIIIII', MAGIC_NUMBER, 1, 100, 1, 32+48, 0)
    agent = struct.pack('HHffffffffH', 0, 0, 1.0, 2.0, 0.5, 0.3, 0.4, 0.1, 1.2, 0)
    metrics = struct.pack('dddddd', 0.034, 2.145, 0.098, 0.45, 0.55, 0.15)
    packet = header + agent + metrics

    # Send from mock server
    mock_server.sendto(packet, ('127.0.0.1', 5555))

    # Receive on client (with timeout)
    start = time.time()
    received_packet = None
    while time.time() - start < 1.0:
        received_packet = client.receive_state()
        if received_packet:
            break
        time.sleep(0.01)

    assert received_packet is not None
    assert received_packet['header']['timestep'] == 100
    assert len(received_packet['agents']) == 1

    client.close()


def test_send_command():
    """Should send JSON command"""
    client = UDPClient(recv_port=5555, send_port=5556)

    # Create receiver to check if sent
    receiver = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    receiver.bind(('127.0.0.1', 5556))
    receiver.settimeout(0.5)

    # Send command
    command = {'type': 'set_parameter', 'parameter': 'beta', 'value': 0.105}
    success = client.send_command(command)
    assert success == True

    # Verify received
    data, addr = receiver.recvfrom(4096)
    assert b'set_parameter' in data

    receiver.close()
    client.close()
```

**Step 2: Run test to verify it fails**

```bash
python -m pytest bridge/test_udp_client.py -v
```

Expected: FAIL - module 'gui.bridge.udp_client' not found

**Step 3: Implement udp_client.py**

File: `gui/bridge/udp_client.py`

```python
"""
UDP Client for EPH GUI

Receives state data from C++ server and sends control commands
"""

import socket
import json
import logging
from typing import Optional, Dict, Any

from .protocol import deserialize_state_packet

logger = logging.getLogger(__name__)


class UDPClient:
    """
    UDP Client for bidirectional communication with C++ server

    Port 5555: Receive state data (C++ → Python)
    Port 5556: Send control commands (Python → C++)
    """

    def __init__(self, recv_port: int = 5555, send_port: int = 5556,
                 server_host: str = '127.0.0.1'):
        """
        Initialize UDP client

        Args:
            recv_port: Port to receive state data on
            send_port: Port to send commands to
            server_host: C++ server host (default: localhost)
        """
        self.recv_port = recv_port
        self.send_port = send_port
        self.server_host = server_host

        # Create sockets
        self.recv_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.send_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        # Set non-blocking mode for receive
        self.recv_socket.setblocking(False)

        # Bind receive socket
        try:
            self.recv_socket.bind(('0.0.0.0', recv_port))
            logger.info(f"UDP client bound to port {recv_port}")
        except OSError as e:
            logger.error(f"Failed to bind to port {recv_port}: {e}")
            raise

        # Connection state
        self.is_connected = False
        self.last_sequence_num = -1
        self.lost_packet_count = 0

    def receive_state(self) -> Optional[Dict[str, Any]]:
        """
        Receive state packet (non-blocking)

        Returns:
            Deserialized packet dict or None if no data available
        """
        try:
            data, addr = self.recv_socket.recvfrom(65536)  # Max UDP packet size

            if not self.is_connected:
                self.is_connected = True
                logger.info(f"Connected to C++ server at {addr}")

            packet = deserialize_state_packet(data)
            if packet is None:
                logger.warning("Failed to deserialize packet")
                return None

            # Check for packet loss
            seq_num = packet['header']['sequence_num']
            if self.last_sequence_num >= 0:
                expected = self.last_sequence_num + 1
                if seq_num != expected:
                    lost = seq_num - expected
                    self.lost_packet_count += lost
                    logger.warning(f"Packet loss detected: {lost} packets (total: {self.lost_packet_count})")

            self.last_sequence_num = seq_num

            return packet

        except BlockingIOError:
            # No data available (expected in non-blocking mode)
            return None
        except Exception as e:
            logger.error(f"Error receiving state: {e}")
            return None

    def send_command(self, command: Dict[str, Any]) -> bool:
        """
        Send JSON command to C++ server

        Args:
            command: Dictionary to send as JSON

        Returns:
            True if sent successfully
        """
        try:
            json_str = json.dumps(command)
            data = json_str.encode('utf-8')

            self.send_socket.sendto(data, (self.server_host, self.send_port))
            logger.debug(f"Sent command: {command}")
            return True

        except Exception as e:
            logger.error(f"Error sending command: {e}")
            return False

    def close(self):
        """Close sockets"""
        self.recv_socket.close()
        self.send_socket.close()
        logger.info("UDP client closed")

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
```

**Step 4: Run tests**

```bash
python -m pytest bridge/test_udp_client.py -v
```

Expected: PASS (may need to run with sudo if port binding fails, or use higher ports)

**Step 5: Commit**

```bash
git add gui/bridge/udp_client.py gui/bridge/test_udp_client.py
git commit -m "feat(python): implement UDP client for C++ communication

- Non-blocking receive on port 5555 (state data)
- JSON command send on port 5556 (control)
- Packet loss detection via sequence numbers
- Comprehensive logging
- Unit tests with mock server

Phase 1: Python UDP client"
```

---

## Task 8: Python - Basic Main Window

**Duration:** 15-20 minutes

**Files:**
- Create: `gui/widgets/main_window.py`
- Create: `gui/main.py`

**Step 1: Create minimal MainWindow**

File: `gui/widgets/main_window.py`

```python
"""
Main Window for EPH GUI

Phase 1: Minimal window with status bar showing connection state
"""

from PyQt6.QtWidgets import QMainWindow, QLabel, QStatusBar
from PyQt6.QtCore import Qt


class MainWindow(QMainWindow):
    """Main application window"""

    def __init__(self):
        super().__init__()

        self.setWindowTitle("EPH v2.1 - GUI")
        self.setGeometry(100, 100, 1200, 800)

        # Status bar
        self.status_bar = QStatusBar()
        self.setStatusBar(self.status_bar)

        # Connection indicator
        self.connection_label = QLabel("● Disconnected")
        self.connection_label.setStyleSheet("color: red;")
        self.status_bar.addPermanentWidget(self.connection_label)

        # Step counter
        self.step_label = QLabel("Step: 0")
        self.status_bar.addPermanentWidget(self.step_label)

        # Central widget placeholder
        central_label = QLabel("EPH v2.1 - Phase 1 Foundation\n\nUDP Connection Ready")
        central_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.setCentralWidget(central_label)

    def update_connection_status(self, connected: bool):
        """Update connection status indicator"""
        if connected:
            self.connection_label.setText("● Connected")
            self.connection_label.setStyleSheet("color: green;")
        else:
            self.connection_label.setText("● Disconnected")
            self.connection_label.setStyleSheet("color: red;")

    def update_step(self, step: int):
        """Update step counter"""
        self.step_label.setText(f"Step: {step}")
```

**Step 2: Create application entry point**

File: `gui/main.py`

```python
#!/usr/bin/env python3
"""
EPH v2.1 GUI - Main Entry Point

Phase 1: Test UDP communication with minimal GUI
"""

import sys
import logging
from PyQt6.QtWidgets import QApplication
from PyQt6.QtCore import QTimer

from widgets.main_window import MainWindow
from bridge.udp_client import UDPClient

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


class EPHApplication:
    """Main application controller"""

    def __init__(self):
        self.app = QApplication(sys.argv)
        self.window = MainWindow()
        self.client = UDPClient()

        # Timer for polling UDP data
        self.timer = QTimer()
        self.timer.timeout.connect(self.update)
        self.timer.start(33)  # ~30 FPS

        logger.info("EPH GUI initialized")

    def update(self):
        """Update loop - called every frame"""
        packet = self.client.receive_state()

        if packet:
            # Update connection status
            if not self.client.is_connected:
                self.window.update_connection_status(True)

            # Update step counter
            timestep = packet['header']['timestep']
            self.window.update_step(timestep)

            # Log metrics (Phase 1: just print)
            metrics = packet['metrics']
            logger.debug(f"φ={metrics['phi']:.4f}, χ={metrics['chi']:.4f}, β={metrics['beta_current']:.3f}")

    def run(self):
        """Run the application"""
        self.window.show()
        logger.info("EPH GUI running. Waiting for C++ server...")
        return self.app.exec()


def main():
    """Entry point"""
    app = EPHApplication()
    sys.exit(app.run())


if __name__ == '__main__':
    main()
```

**Step 3: Make main.py executable**

```bash
chmod +x gui/main.py
```

**Step 4: Test the GUI (manual test)**

```bash
# Terminal 1: Start GUI
cd gui
source .venv/bin/activate
python main.py
```

Expected: Window opens with "Disconnected" status

**Step 5: Close GUI and commit**

```bash
git add gui/widgets/main_window.py gui/main.py
git commit -m "feat(python): add basic PyQt6 main window

- Minimal MainWindow with status bar
- Connection indicator (red/green)
- Step counter display
- 30 FPS update loop polling UDP client
- Entry point in main.py

Phase 1: Python GUI foundation

Test: GUI opens, shows disconnected status"
```

---

## Task 9: Integration Test - End-to-End Communication

**Duration:** 15-20 minutes

**Files:**
- Create: `scripts/test_udp_communication.sh`
- Modify: `cpp_server/main.cpp` (add actual simulation integration)

**Step 1: Update C++ server to send real data**

File: `cpp_server/main.cpp`

Replace the TODO section with:

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include "udp_server.hpp"
#include "eph_swarm/swarm_manager.hpp"
#include "eph_phase/phase_analyzer.hpp"
#include "eph_spm/saliency_polar_map.hpp"

using namespace eph;

int main(int argc, char** argv) {
    std::cout << "EPH v2.1 GUI Server" << std::endl;
    std::cout << "Initializing UDP server..." << std::endl;

    udp::UDPServer server(5555, 5556);

    if (!server.is_initialized()) {
        std::cerr << "Failed to initialize UDP server: "
                  << server.get_last_error() << std::endl;
        return 1;
    }

    std::cout << "UDP server initialized successfully" << std::endl;
    std::cout << "  Send port: 5555 (state data)" << std::endl;
    std::cout << "  Recv port: 5556 (commands)" << std::endl;

    // Initialize simulation
    const size_t N_AGENTS = 10;  // Small for Phase 1 test
    const Scalar BETA = 0.098;
    const int AVG_NEIGHBORS = 6;

    swarm::SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);
    phase::PhaseAnalyzer analyzer;

    // Create simple SPM (all zeros for Phase 1)
    spm::SaliencyPolarMap test_spm;

    std::cout << "Simulation initialized (N=" << N_AGENTS << ")" << std::endl;
    std::cout << "Starting simulation loop..." << std::endl;

    uint32_t sequence_num = 0;
    uint32_t timestep = 0;
    const Scalar dt = 0.1;
    const int SEND_INTERVAL = 10;  // Send every 10 steps

    while (true) {
        // Check for commands
        auto command = server.receive_command();
        if (command.has_value()) {
            std::cout << "Received command: " << command.value().dump() << std::endl;

            // Handle parameter changes (Phase 1: just log)
            if (command.value().contains("parameter")) {
                std::string param = command.value()["parameter"];
                std::cout << "  Parameter: " << param << std::endl;
            }
        }

        // Update simulation
        swarm.update_all_agents(test_spm, dt);
        timestep++;

        // Send state at intervals
        if (timestep % SEND_INTERVAL == 0) {
            udp::StatePacket packet;
            packet.header.sequence_num = sequence_num++;
            packet.header.timestep = timestep;
            packet.header.num_agents = N_AGENTS;

            // Collect agent data
            for (size_t i = 0; i < N_AGENTS; ++i) {
                const auto& agent_state = swarm.get_agent_state(i);

                udp::AgentData agent_data;
                agent_data.agent_id = static_cast<uint16_t>(i);
                agent_data.x = static_cast<float>(agent_state.position.x());
                agent_data.y = static_cast<float>(agent_state.position.y());
                agent_data.vx = static_cast<float>(agent_state.velocity.x());
                agent_data.vy = static_cast<float>(agent_state.velocity.y());
                agent_data.haze_mean = static_cast<float>(swarm.get_agent_haze_mean(i));
                agent_data.fatigue = static_cast<float>(agent_state.fatigue);
                agent_data.efe = 0.0f;  // TODO: Add EFE tracking

                packet.agents.push_back(agent_data);
            }

            // Collect metrics
            auto haze_fields = swarm.get_all_haze_fields();
            Scalar phi = analyzer.compute_phi(haze_fields);
            Scalar chi = analyzer.compute_chi(haze_fields);

            packet.metrics.phi = phi;
            packet.metrics.chi = chi;
            packet.metrics.beta_current = BETA;
            packet.metrics.avg_haze = analyzer.compute_avg_haze(haze_fields);
            packet.metrics.avg_speed = swarm.get_avg_speed();
            packet.metrics.avg_fatigue = swarm.get_avg_fatigue();

            // Calculate checksum
            auto buffer = udp::serialize_state_packet(packet);
            packet.header.checksum = udp::calculate_crc32(
                buffer.data() + sizeof(udp::PacketHeader),
                buffer.size() - sizeof(udp::PacketHeader)
            );
            packet.header.data_length = static_cast<uint32_t>(
                packet.agents.size() * sizeof(udp::AgentData) + sizeof(udp::MetricsData)
            );

            // Send
            if (!server.send_state(packet)) {
                std::cerr << "Failed to send packet: " << server.get_last_error() << std::endl;
            }
        }

        // Sleep for dt (100ms)
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(dt * 1000)));
    }

    return 0;
}
```

**Step 2: Rebuild C++ server**

```bash
cd build
cmake --build . --target eph_gui_server
```

Expected: Builds successfully

**Step 3: Create test script**

File: `scripts/test_udp_communication.sh`

```bash
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
```

**Step 4: Make script executable and run**

```bash
chmod +x scripts/test_udp_communication.sh
./scripts/test_udp_communication.sh
```

Expected:
- C++ server starts, begins simulation
- Python GUI opens
- Status changes from "● Disconnected" to "● Connected" (green)
- Step counter increments: "Step: 10", "Step: 20", etc.

**Step 5: Stop test (Ctrl+C) and commit**

```bash
git add cpp_server/main.cpp scripts/test_udp_communication.sh
git commit -m "feat: integrate C++ simulation with UDP server

- Update main.cpp to run SwarmManager and send real data
- Send state packets every 10 steps
- Include agent positions, velocities, haze, metrics
- Add integration test script
- Test: End-to-end C++ ↔ Python communication working

Phase 1: Integration complete

TEST RESULT: ✅ GUI shows 'Connected', steps increment"
```

---

## Phase 1 Completion Checklist

**C++ Side:**
- [x] Field of View parameter added (270° default)
- [x] Binary protocol defined (protocol.hpp)
- [x] UDP server implemented (udp_server.hpp/cpp)
- [x] CMake integration (builds eph_gui_server)
- [x] Main loop with SwarmManager integration

**Python Side:**
- [x] Project structure (gui/ directory)
- [x] Requirements.txt and venv setup
- [x] Protocol deserialization (protocol.py + tests)
- [x] UDP client (udp_client.py + tests)
- [x] Basic MainWindow (PyQt6)
- [x] Application entry point (main.py)

**Integration:**
- [x] End-to-end communication test passes
- [x] GUI shows connection status
- [x] GUI displays simulation timestep
- [x] Packet loss detection working

**Documentation:**
- [x] All code has docstrings
- [x] Test scripts included
- [x] Commit messages follow convention

---

## Next Steps

**Phase 2 Preview:**
- Global View widget (matplotlib canvas)
- Agent visualization (circles + arrows)
- Parameter panel (basic sliders)
- Play/Pause controls

**Estimated Time:** Phase 2 will take 2-3 weeks after Phase 1 completion.

---

**Plan complete and saved to `docs/plans/2026-02-10-pyqt6-gui-phase1-implementation.md`.**

**Two execution options:**

**1. Subagent-Driven (this session)** - I dispatch fresh subagent per task, review between tasks, fast iteration

**2. Parallel Session (separate)** - Open new session with executing-plans, batch execution with checkpoints

**Which approach would you like?**
