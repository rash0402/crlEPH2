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

            # Note: is_connected is managed by main.py to coordinate with GUI updates
            if not self.is_connected:
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
