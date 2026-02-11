"""
UDP Binary Protocol for EPH GUI Communication

Matches C++ protocol definition in cpp_server/protocol.hpp
"""

import struct
from typing import Optional, Dict, List, Any

# Magic number: 0xEFE20210 (EPH v2.1, 2021-0)
MAGIC_NUMBER = 0xEFE20210


def calculate_crc32(data: bytes) -> int:
    """CRC32 checksum matching C++ implementation"""
    crc = 0xFFFFFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)))
    return ~crc & 0xFFFFFFFF


class PacketHeader:
    """Binary packet header (24 bytes)"""
    SIZE = 24
    FORMAT = '<IIIIII'  # 6 uint32_t (little-endian)

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
    # uint16 id, uint16 pad, 7x float32 (little-endian)
    FORMAT = '<HHfffffff'

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
    FORMAT = '<dddddd'  # 6 double (little-endian)

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

    # Validate checksum
    num_agents = header['num_agents']
    expected_payload_size = num_agents * AgentData.SIZE + MetricsData.SIZE
    if header['data_length'] != expected_payload_size:
        return None  # Payload size mismatch

    payload_start = PacketHeader.SIZE
    payload_end = payload_start + header['data_length']
    if payload_end > len(data):
        return None

    expected_checksum = calculate_crc32(data[payload_start:payload_end])
    if header['checksum'] != expected_checksum:
        return None  # Checksum validation failed

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
