#ifndef EPH_UDP_PROTOCOL_HPP
#define EPH_UDP_PROTOCOL_HPP

#include <cstdint>
#include <vector>
#include <cstring>
#include "eph_core/types.hpp"

// Protocol uses little-endian byte order (host byte order on most modern systems)
// TODO: Add explicit endianness conversion for cross-platform compatibility

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
} __attribute__((packed));

static_assert(sizeof(PacketHeader) == 24, "PacketHeader must be 24 bytes");

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
} __attribute__((packed));

static_assert(sizeof(AgentData) == 32, "AgentData must be 32 bytes");

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
} __attribute__((packed));

static_assert(sizeof(MetricsData) == 48, "MetricsData must be 48 bytes");

/**
 * @brief Agent detail data (608 bytes)
 *
 * Contains SPM (12x12) and neighbor information for selected agent
 */
struct AgentDetailData {
    uint16_t agent_id;       // Selected agent ID
    uint16_t num_neighbors;  // Number of valid neighbors
    float spm_data[144];     // SPM 12x12 = 144 floats
    float velocity_angle;    // Agent heading in radians
    uint16_t neighbor_ids[6];// Neighbor agent IDs
    uint16_t padding[6];     // Padding to match Python format (12 bytes)
} __attribute__((packed));

static_assert(sizeof(AgentDetailData) == 608, "AgentDetailData must be 608 bytes");

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

    // Copy header (will update checksum later)
    std::memcpy(buffer.data() + offset, &packet.header, sizeof(PacketHeader));
    offset += sizeof(PacketHeader);

    // Copy agent data
    for (const auto& agent : packet.agents) {
        std::memcpy(buffer.data() + offset, &agent, sizeof(AgentData));
        offset += sizeof(AgentData);
    }

    // Copy metrics
    std::memcpy(buffer.data() + offset, &packet.metrics, sizeof(MetricsData));

    // Calculate checksum and data_length of payload (after copying data to buffer)
    PacketHeader header_with_checksum = packet.header;
    header_with_checksum.data_length = static_cast<uint32_t>(
        packet.agents.size() * sizeof(AgentData) + sizeof(MetricsData)
    );
    header_with_checksum.checksum = calculate_crc32(
        buffer.data() + sizeof(PacketHeader),
        buffer.size() - sizeof(PacketHeader)
    );

    // Re-copy header with updated checksum and data_length
    std::memcpy(buffer.data(), &header_with_checksum, sizeof(PacketHeader));

    return buffer;
}

} // namespace eph::udp

#endif // EPH_UDP_PROTOCOL_HPP
