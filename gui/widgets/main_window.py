"""
Main Window for EPH GUI

Phase 2: Add Global View Panel
"""

from PyQt6.QtWidgets import QMainWindow, QLabel, QStatusBar
from PyQt6.QtCore import Qt
from typing import List, Dict, Any

from .global_view import GlobalViewWidget


class MainWindow(QMainWindow):
    """Main application window"""

    def __init__(self):
        super().__init__()

        self.setWindowTitle("EPH v2.1 - GUI")
        self.setGeometry(100, 100, 1200, 800)

        # Central widget: Global View
        self.global_view = GlobalViewWidget()
        self.setCentralWidget(self.global_view)

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

        # FPS counter (Phase 2)
        self.fps_label = QLabel("FPS: 0.0")
        self.status_bar.addPermanentWidget(self.fps_label)

        # Packet loss counter (Phase 2)
        self.packet_loss_label = QLabel("Loss: 0")
        self.status_bar.addPermanentWidget(self.packet_loss_label)

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

    def update_agents(self, agents: List[Dict[str, Any]]):
        """Update agent visualization"""
        self.global_view.update_agents(agents)

    def update_fps(self, fps: float):
        """Update FPS display"""
        self.fps_label.setText(f"FPS: {fps:.1f}")

    def update_packet_loss(self, count: int):
        """Update packet loss display"""
        color = "red" if count > 0 else "green"
        self.packet_loss_label.setText(f"Loss: {count}")
        self.packet_loss_label.setStyleSheet(f"color: {color};")
