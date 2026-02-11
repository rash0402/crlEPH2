import struct
import pytest
from gui.bridge.protocol import (
    MAGIC_NUMBER,
    PacketHeader,
    AgentData,
    MetricsData,
    deserialize_state_packet,
    calculate_crc32
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
    # Agent 0
    agent0_data = struct.pack(
        '<HHfffffff',
        0,      # agent_id
        0,      # padding
        1.0,    # x
        2.0,    # y
        0.5,    # vx
        0.3,    # vy
        0.4,    # haze_mean
        0.1,    # fatigue
        1.2     # efe
    )

    # Agent 1
    agent1_data = struct.pack(
        '<HHfffffff',
        1, 0, 3.0, 4.0, 0.2, 0.1, 0.5, 0.2, 1.5
    )

    # Metrics
    metrics_data = struct.pack(
        '<dddddd',
        0.034,  # phi
        2.145,  # chi
        0.098,  # beta_current
        0.45,   # avg_haze
        0.55,   # avg_speed
        0.15    # avg_fatigue
    )

    # Calculate checksum of payload
    payload = agent0_data + agent1_data + metrics_data
    checksum = calculate_crc32(payload)

    # Create header with correct checksum
    header_data = struct.pack(
        '<IIIIII',
        MAGIC_NUMBER,  # magic_number
        1,             # sequence_num
        100,           # timestep
        2,             # num_agents
        2 * 32 + 48,   # data_length (2 agents + metrics)
        checksum       # checksum
    )

    packet_bytes = header_data + payload

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
    bad_header = struct.pack('<IIIIII', 0xDEADBEEF, 0, 0, 0, 0, 0)
    packet = deserialize_state_packet(bad_header)
    assert packet is None


def test_deserialize_truncated_packet():
    """Should return None for truncated packet"""
    truncated = struct.pack('<III', MAGIC_NUMBER, 1, 100)
    packet = deserialize_state_packet(truncated)
    assert packet is None


def test_deserialize_invalid_checksum():
    """Should return None for invalid checksum"""
    # Create payload
    agent_data = struct.pack('<HHfffffff', 0, 0, 1.0, 2.0, 0.5, 0.3, 0.4, 0.1, 1.2)
    metrics_data = struct.pack('<dddddd', 0.034, 2.145, 0.098, 0.45, 0.55, 0.15)
    payload = agent_data + metrics_data

    # Create header with WRONG checksum
    header_data = struct.pack(
        '<IIIIII',
        MAGIC_NUMBER,
        1,
        100,
        1,  # num_agents
        len(payload),
        0xDEADBEEF  # Wrong checksum
    )

    packet_bytes = header_data + payload
    packet = deserialize_state_packet(packet_bytes)
    assert packet is None


def test_deserialize_payload_size_mismatch():
    """Should return None when header data_length doesn't match expected payload size"""
    # Create payload for 1 agent
    agent_data = struct.pack('<HHfffffff', 0, 0, 1.0, 2.0, 0.5, 0.3, 0.4, 0.1, 1.2)
    metrics_data = struct.pack('<dddddd', 0.034, 2.145, 0.098, 0.45, 0.55, 0.15)
    payload = agent_data + metrics_data

    # But header claims 2 agents (wrong data_length)
    header_data = struct.pack(
        '<IIIIII',
        MAGIC_NUMBER,
        1,
        100,
        1,  # num_agents = 1
        2 * 32 + 48,  # data_length claims 2 agents (wrong!)
        0
    )

    packet_bytes = header_data + payload
    packet = deserialize_state_packet(packet_bytes)
    assert packet is None
