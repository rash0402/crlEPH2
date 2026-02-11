"""
Parameter Panel Widget

Controls for runtime parameter adjustment:
- Beta (β): Markov blanket breaking parameter
- Learning rate (η)
- Number of agents (N)
"""

from PyQt6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout,
                             QLabel, QSlider, QDoubleSpinBox, QSpinBox,
                             QPushButton, QGroupBox)
from PyQt6.QtCore import Qt, pyqtSignal


class ParameterPanel(QWidget):
    """Parameter control panel"""

    # Signal emitted when parameters should be applied
    parameters_changed = pyqtSignal(dict)  # Dict: {'beta': 0.098, 'eta': 0.01, 'n_agents': 10}

    def __init__(self, parent=None):
        super().__init__(parent)

        # Initial values (from Phase 1 defaults)
        self.beta_value = 0.098
        self.eta_value = 0.01
        self.n_agents_value = 10

        self._setup_ui()

    def _setup_ui(self):
        """Create UI layout"""
        layout = QVBoxLayout()

        # Beta control group
        beta_group = self._create_beta_group()
        layout.addWidget(beta_group)

        # Eta control group
        eta_group = self._create_eta_group()
        layout.addWidget(eta_group)

        # N agents control group
        n_agents_group = self._create_n_agents_group()
        layout.addWidget(n_agents_group)

        # Apply/Reset buttons
        button_layout = QHBoxLayout()

        self.apply_button = QPushButton("Apply")
        self.apply_button.clicked.connect(self._on_apply)

        self.reset_button = QPushButton("Reset")
        self.reset_button.clicked.connect(self._on_reset)

        button_layout.addWidget(self.apply_button)
        button_layout.addWidget(self.reset_button)

        layout.addLayout(button_layout)
        layout.addStretch()

        self.setLayout(layout)

    def _create_beta_group(self) -> QGroupBox:
        """Create beta parameter control group"""
        group = QGroupBox("β (Beta) - MB Breaking")
        layout = QVBoxLayout()

        # Slider (0.01 to 0.20, step 0.001)
        slider_layout = QHBoxLayout()
        self.beta_slider = QSlider(Qt.Orientation.Horizontal)
        self.beta_slider.setMinimum(10)   # 0.010
        self.beta_slider.setMaximum(200)  # 0.200
        self.beta_slider.setValue(98)     # 0.098
        self.beta_slider.setTickInterval(10)
        self.beta_slider.setTickPosition(QSlider.TickPosition.TicksBelow)
        self.beta_slider.valueChanged.connect(self._on_beta_slider_changed)

        # SpinBox
        self.beta_spinbox = QDoubleSpinBox()
        self.beta_spinbox.setRange(0.01, 0.20)
        self.beta_spinbox.setSingleStep(0.001)
        self.beta_spinbox.setDecimals(3)
        self.beta_spinbox.setValue(0.098)
        self.beta_spinbox.valueChanged.connect(self._on_beta_spinbox_changed)

        slider_layout.addWidget(self.beta_slider, 3)
        slider_layout.addWidget(self.beta_spinbox, 1)

        # Info label
        info_label = QLabel("Critical point β_c ≈ 0.098")
        info_label.setStyleSheet("color: gray; font-size: 10px;")

        layout.addLayout(slider_layout)
        layout.addWidget(info_label)

        group.setLayout(layout)
        return group

    def _create_eta_group(self) -> QGroupBox:
        """Create eta (learning rate) control group"""
        group = QGroupBox("η (Eta) - Learning Rate")
        layout = QHBoxLayout()

        self.eta_spinbox = QDoubleSpinBox()
        self.eta_spinbox.setRange(0.001, 0.1)
        self.eta_spinbox.setSingleStep(0.001)
        self.eta_spinbox.setDecimals(3)
        self.eta_spinbox.setValue(0.01)
        self.eta_spinbox.valueChanged.connect(self._on_eta_changed)

        layout.addWidget(QLabel("η:"))
        layout.addWidget(self.eta_spinbox)
        layout.addStretch()

        group.setLayout(layout)
        return group

    def _create_n_agents_group(self) -> QGroupBox:
        """Create N agents control group"""
        group = QGroupBox("N - Number of Agents")
        layout = QHBoxLayout()

        self.n_agents_spinbox = QSpinBox()
        self.n_agents_spinbox.setRange(5, 200)
        self.n_agents_spinbox.setSingleStep(5)
        self.n_agents_spinbox.setValue(10)
        self.n_agents_spinbox.valueChanged.connect(self._on_n_agents_changed)

        info_label = QLabel("(Requires restart)")
        info_label.setStyleSheet("color: gray; font-size: 10px;")

        layout.addWidget(QLabel("N:"))
        layout.addWidget(self.n_agents_spinbox)
        layout.addWidget(info_label)
        layout.addStretch()

        group.setLayout(layout)
        return group

    def _on_beta_slider_changed(self, value: int):
        """Beta slider changed"""
        beta = value / 1000.0
        self.beta_value = beta
        self.beta_spinbox.blockSignals(True)
        self.beta_spinbox.setValue(beta)
        self.beta_spinbox.blockSignals(False)

    def _on_beta_spinbox_changed(self, value: float):
        """Beta spinbox changed"""
        self.beta_value = value
        slider_val = int(value * 1000)
        self.beta_slider.blockSignals(True)
        self.beta_slider.setValue(slider_val)
        self.beta_slider.blockSignals(False)

    def _on_eta_changed(self, value: float):
        """Eta changed"""
        self.eta_value = value

    def _on_n_agents_changed(self, value: int):
        """N agents changed"""
        self.n_agents_value = value

    def _on_apply(self):
        """Apply button clicked"""
        params = {
            'beta': self.beta_value,
            'eta': self.eta_value,
            'n_agents': self.n_agents_value
        }
        self.parameters_changed.emit(params)

    def _on_reset(self):
        """Reset button clicked"""
        self.beta_spinbox.setValue(0.098)
        self.eta_spinbox.setValue(0.01)
        self.n_agents_spinbox.setValue(10)
