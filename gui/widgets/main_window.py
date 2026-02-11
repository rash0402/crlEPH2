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
