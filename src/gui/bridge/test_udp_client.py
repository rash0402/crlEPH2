import socket
import struct
import time
import pytest
from gui.bridge.udp_client import UDPClient
from gui.bridge.protocol import MAGIC_NUMBER, calculate_crc32


@pytest.fixture
def mock_server():
    """Create a mock C++ server for testing"""
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Don't bind - we'll just use it to send
    yield server_socket
    server_socket.close()


def test_udp_client_initialization():
    """Client should initialize with correct ports"""
    with UDPClient(recv_port=5557, send_port=5558) as client:
        assert client.recv_port == 5557
        assert client.send_port == 5558
        assert client.is_connected == False  # Not connected until receive data


def test_receive_state_packet(mock_server):
    """Should receive and parse state packet"""
    # Use unique port for this test
    recv_port = 5559
    with UDPClient(recv_port=recv_port, send_port=5560) as client:
        # Create minimal valid packet - AgentData format: '<HHfffffff' (uint16 id, uint16 pad, 7x float32)
        agent = struct.pack('<HHfffffff', 0, 0, 1.0, 2.0, 0.5, 0.3, 0.4, 0.1, 1.2)
        metrics = struct.pack('<dddddd', 0.034, 2.145, 0.098, 0.45, 0.55, 0.15)
        payload = agent + metrics
        checksum = calculate_crc32(payload)
        header = struct.pack('<IIIIII', MAGIC_NUMBER, 1, 100, 1, len(payload), checksum)
        packet = header + payload

        # Send from mock server to client
        mock_server.sendto(packet, ('127.0.0.1', recv_port))

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


def test_send_command():
    """Should send JSON command"""
    # Use unique ports for this test
    recv_port = 5561
    send_port = 5562
    with UDPClient(recv_port=recv_port, send_port=send_port) as client:
        # Create receiver to check if sent
        receiver = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        receiver.bind(('127.0.0.1', send_port))
        receiver.settimeout(0.5)

        # Send command
        command = {'type': 'set_parameter', 'parameter': 'beta', 'value': 0.105}
        success = client.send_command(command)
        assert success == True

        # Verify received
        data, addr = receiver.recvfrom(4096)
        assert b'set_parameter' in data

        receiver.close()
