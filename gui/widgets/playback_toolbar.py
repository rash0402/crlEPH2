"""
Playback Control Toolbar

Controls:
- Play/Pause/Stop buttons
- Speed selector (x0.5, x1, x2, x5)
"""

from PyQt6.QtWidgets import QToolBar, QPushButton, QComboBox, QLabel
from PyQt6.QtCore import pyqtSignal


class PlaybackToolbar(QToolBar):
    """Toolbar for playback controls"""

    # Signals
    play_clicked = pyqtSignal()
    pause_clicked = pyqtSignal()
    stop_clicked = pyqtSignal()
    speed_changed = pyqtSignal(float)  # Speed multiplier

    def __init__(self, parent=None):
        super().__init__("Playback Controls", parent)

        self.is_playing = False
        self.current_speed = 1.0

        self._setup_ui()

    def _setup_ui(self):
        """Create toolbar buttons and widgets"""
        # Play button
        self.play_button = QPushButton("▶ Play")
        self.play_button.clicked.connect(self._on_play)
        self.addWidget(self.play_button)

        # Pause button
        self.pause_button = QPushButton("⏸ Pause")
        self.pause_button.clicked.connect(self._on_pause)
        self.pause_button.setEnabled(False)
        self.addWidget(self.pause_button)

        # Stop button
        self.stop_button = QPushButton("⏹ Stop")
        self.stop_button.clicked.connect(self._on_stop)
        self.addWidget(self.stop_button)

        self.addSeparator()

        # Speed control
        self.addWidget(QLabel("Speed:"))
        self.speed_combo = QComboBox()
        self.speed_combo.addItems(["x0.5", "x1", "x2", "x5"])
        self.speed_combo.setCurrentText("x1")
        self.speed_combo.currentTextChanged.connect(self._on_speed_changed)
        self.addWidget(self.speed_combo)

    def _on_play(self):
        """Play button clicked"""
        self.is_playing = True
        self.play_button.setEnabled(False)
        self.pause_button.setEnabled(True)
        self.play_clicked.emit()

    def _on_pause(self):
        """Pause button clicked"""
        self.is_playing = False
        self.play_button.setEnabled(True)
        self.pause_button.setEnabled(False)
        self.pause_clicked.emit()

    def _on_stop(self):
        """Stop button clicked"""
        self.is_playing = False
        self.play_button.setEnabled(True)
        self.pause_button.setEnabled(False)
        self.stop_clicked.emit()

    def _on_speed_changed(self, text: str):
        """Speed combo changed"""
        # Parse "x1.5" -> 1.5
        speed = float(text.replace('x', ''))
        self.current_speed = speed
        self.speed_changed.emit(speed)
