"""
Agent Detail Panel - 3-column layout for SPM visualization

Displays:
- Left: SPM Heatmap (12x12 matrix)
- Center: SPM Polar (circular 270° view)
- Right: Statistics (position, velocity, neighbors)
"""

from PyQt6.QtWidgets import QWidget, QHBoxLayout, QVBoxLayout, QLabel
from PyQt6.QtCore import Qt
from typing import Dict, Any, Optional
import numpy as np


class AgentDetailPanel(QWidget):
    """Widget for displaying selected agent details"""

    def __init__(self, parent=None):
        super().__init__(parent)

        # Main horizontal layout (3 columns)
        main_layout = QHBoxLayout()
        main_layout.setContentsMargins(5, 5, 5, 5)
        main_layout.setSpacing(10)

        # Column 1: SPM Heatmap
        self.heatmap_widget = QLabel("SPM Heatmap\n(12×12)")
        self.heatmap_widget.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.heatmap_widget.setMinimumSize(300, 300)
        self.heatmap_widget.setStyleSheet("border: 1px solid gray;")
        main_layout.addWidget(self.heatmap_widget)

        # Column 2: SPM Polar
        self.polar_widget = QLabel("SPM Polar\n(270° FOV)")
        self.polar_widget.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.polar_widget.setMinimumSize(300, 300)
        self.polar_widget.setStyleSheet("border: 1px solid gray;")
        main_layout.addWidget(self.polar_widget)

        # Column 3: Statistics
        stats_layout = QVBoxLayout()
        self.stats_label = QLabel("No agent selected")
        self.stats_label.setAlignment(Qt.AlignmentFlag.AlignTop | Qt.AlignmentFlag.AlignLeft)
        self.stats_label.setWordWrap(True)
        stats_layout.addWidget(self.stats_label)

        stats_container = QWidget()
        stats_container.setLayout(stats_layout)
        stats_container.setMinimumWidth(200)
        stats_container.setStyleSheet("border: 1px solid gray;")
        main_layout.addWidget(stats_container)

        self.setLayout(main_layout)

        # Current data
        self.current_detail: Optional[Dict[str, Any]] = None

    def update_detail(self, detail: Dict[str, Any]):
        """Update panel with agent detail data"""
        self.current_detail = detail

        # Update statistics text
        agent_id = detail.get('agent_id', -1)
        spm_shape = detail.get('spm', np.zeros((12, 12))).shape
        angle = detail.get('velocity_angle', 0.0)
        neighbors = detail.get('neighbor_ids', [])

        stats_text = f"""<b>Agent #{agent_id}</b><br><br>
        <b>SPM Data:</b><br>
        Shape: {spm_shape[0]}×{spm_shape[1]}<br>
        Min: {detail.get('spm', np.zeros((12, 12))).min():.3f}<br>
        Max: {detail.get('spm', np.zeros((12, 12))).max():.3f}<br><br>
        <b>Heading:</b><br>
        {np.degrees(angle):.1f}°<br><br>
        <b>Neighbors ({len(neighbors)}):</b><br>
        {', '.join(f'#{n}' for n in neighbors)}
        """

        self.stats_label.setText(stats_text)

        # TODO: Update heatmap (Task 4)
        # TODO: Update polar plot (Task 5)

    def clear(self):
        """Clear all displays"""
        self.current_detail = None
        self.stats_label.setText("No agent selected")
        self.heatmap_widget.setText("SPM Heatmap\n(12×12)")
        self.polar_widget.setText("SPM Polar\n(270° FOV)")
