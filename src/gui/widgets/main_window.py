"""
Main Window for EPH GUI

Phase 2: Add Global View Panel
"""

from PyQt6.QtWidgets import QMainWindow, QLabel, QStatusBar, QDockWidget
from PyQt6.QtCore import Qt, pyqtSignal
from typing import List, Dict, Any

from .global_view import GlobalViewWidget
from .parameter_panel import ParameterPanel
from .playback_toolbar import PlaybackToolbar


class MainWindow(QMainWindow):
    """Main application window"""

    # Signal for parameter changes
    parameters_changed = pyqtSignal(dict)

    # Playback control signals (Phase 2)
    playback_play = pyqtSignal()
    playback_pause = pyqtSignal()
    playback_stop = pyqtSignal()
    playback_speed_changed = pyqtSignal(float)

    def __init__(self):
        super().__init__()

        self.setWindowTitle("EPH v2.1 - GUI")
        self.setGeometry(100, 100, 1200, 800)

        # Toolbar: Playback Controls (Phase 2)
        self.playback_toolbar = PlaybackToolbar()
        self.addToolBar(self.playback_toolbar)

        # Connect toolbar signals
        self.playback_toolbar.play_clicked.connect(lambda: self.playback_play.emit())
        self.playback_toolbar.pause_clicked.connect(lambda: self.playback_pause.emit())
        self.playback_toolbar.stop_clicked.connect(lambda: self.playback_stop.emit())
        self.playback_toolbar.speed_changed.connect(lambda s: self.playback_speed_changed.emit(s))

        # Central widget: Global View
        self.global_view = GlobalViewWidget()
        self.setCentralWidget(self.global_view)

        # Left dock: Parameter Panel
        self._create_parameter_dock()

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

        # Metrics display (Phase 2)
        self.status_bar.addPermanentWidget(QLabel(" | "))  # Separator
        self.phi_label = QLabel("φ: -")
        self.status_bar.addPermanentWidget(self.phi_label)
        self.chi_label = QLabel("χ: -")
        self.status_bar.addPermanentWidget(self.chi_label)
        self.haze_label = QLabel("Haze: -")
        self.status_bar.addPermanentWidget(self.haze_label)
        self.beta_label = QLabel("β: -")
        self.status_bar.addPermanentWidget(self.beta_label)

    def _create_parameter_dock(self):
        """Create parameter panel dock widget"""
        self.parameter_panel = ParameterPanel()

        dock = QDockWidget("Parameters", self)
        dock.setWidget(self.parameter_panel)
        dock.setAllowedAreas(Qt.DockWidgetArea.LeftDockWidgetArea |
                            Qt.DockWidgetArea.RightDockWidgetArea)

        self.addDockWidget(Qt.DockWidgetArea.LeftDockWidgetArea, dock)

        # Connect signal
        self.parameter_panel.parameters_changed.connect(self._on_parameters_changed)

    def _on_parameters_changed(self, params: Dict[str, Any]):
        """Parameter panel Apply clicked"""
        # Forward to main application
        self.parameters_changed.emit(params)

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

    def update_metrics(self, metrics: Dict[str, float]):
        """Update metrics display (φ, χ, avg_haze, β)"""
        self.phi_label.setText(f"φ: {metrics['phi']:.3f}")
        self.chi_label.setText(f"χ: {metrics['chi']:.3f}")
        self.haze_label.setText(f"Haze: {metrics['avg_haze']:.3f}")
        self.beta_label.setText(f"β: {metrics['beta_current']:.3f}")
