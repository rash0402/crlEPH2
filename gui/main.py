#!/usr/bin/env python3
"""
EPH v2.1 GUI - Main Entry Point

Phase 1: Test UDP communication with minimal GUI
"""

import sys
import logging
import time
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

        # FPS tracking (Phase 2)
        self.last_fps_update = time.time()
        self.frame_count = 0
        self.current_fps = 0.0

        logger.info("EPH GUI initialized")

    def update(self):
        """Update loop - called every frame"""
        packet = self.client.receive_state()

        if packet:
            # Update connection status (Phase 1: once connected, stays connected)
            if not self.client.is_connected:
                self.client.is_connected = True
                self.window.update_connection_status(True)
                logger.info("Connected to C++ server")

            # Update step counter
            timestep = packet['header']['timestep']
            self.window.update_step(timestep)

            # Update agent visualization (Phase 2)
            agents = packet['agents']
            self.window.update_agents(agents)

            # Log metrics (Phase 1: debug level)
            metrics = packet['metrics']
            logger.debug(f"φ={metrics['phi']:.4f}, χ={metrics['chi']:.4f}, β={metrics['beta_current']:.3f}")

        # Update FPS counter (every second)
        self.frame_count += 1
        now = time.time()
        elapsed = now - self.last_fps_update
        if elapsed >= 1.0:
            self.current_fps = self.frame_count / elapsed
            self.window.update_fps(self.current_fps)

            # Update packet loss
            packet_loss = self.client.lost_packet_count
            self.window.update_packet_loss(packet_loss)

            self.frame_count = 0
            self.last_fps_update = now

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
