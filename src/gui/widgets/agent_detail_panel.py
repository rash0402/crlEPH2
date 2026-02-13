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
import logging

import matplotlib
matplotlib.use('Qt5Agg')  # Must be before importing pyplot
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg
from matplotlib.figure import Figure

logger = logging.getLogger(__name__)


class AgentDetailPanel(QWidget):
    """Widget for displaying selected agent details"""

    # Default placeholder text for each column
    _HEATMAP_PLACEHOLDER = "SPM Heatmap\n(12\u00d712)"
    _POLAR_PLACEHOLDER = "SPM Polar\n(270\u00b0 FOV)"

    def __init__(self, parent=None):
        super().__init__(parent)

        # Main horizontal layout (3 columns)
        main_layout = QHBoxLayout()
        main_layout.setContentsMargins(5, 5, 5, 5)
        main_layout.setSpacing(10)

        # Column 1: SPM Heatmap (matplotlib)
        self.heatmap_figure = Figure(figsize=(4, 4), dpi=80)
        self.heatmap_canvas = FigureCanvasQTAgg(self.heatmap_figure)
        self.heatmap_ax = self.heatmap_figure.add_subplot(111)
        self.heatmap_ax.set_title("SPM Heatmap (12×12)")
        self.heatmap_ax.set_xlabel("Angular Bin (θ)")
        self.heatmap_ax.set_ylabel("Radial Bin (r)")
        self.heatmap_figure.tight_layout()
        main_layout.addWidget(self.heatmap_canvas)

        # Initialize colorbar reference
        self.heatmap_colorbar = None

        # Column 2: SPM Polar (matplotlib polar plot)
        self.polar_figure = Figure(figsize=(4, 4), dpi=80)
        self.polar_canvas = FigureCanvasQTAgg(self.polar_figure)
        self.polar_ax = self.polar_figure.add_subplot(111, projection='polar')
        self.polar_ax.set_title("SPM Polar (270° FOV)")
        self.polar_figure.tight_layout()
        main_layout.addWidget(self.polar_canvas)

        # Initialize colorbar reference
        self.polar_colorbar = None

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

    @staticmethod
    def _create_placeholder(text: str, min_size: tuple = (300, 300)) -> QLabel:
        """Create a bordered placeholder label for visualization columns"""
        label = QLabel(text)
        label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        label.setMinimumSize(*min_size)
        label.setStyleSheet("border: 1px solid gray;")
        return label

    def update_detail(self, detail: Dict[str, Any]):
        """Update panel with agent detail data

        Args:
            detail: Dict with keys:
                - agent_id: int
                - spm: np.ndarray (12x12)
                - velocity_angle: float (radians)
                - neighbor_ids: List[int]
        """
        try:
            self.current_detail = detail

            # Extract data once
            agent_id = detail.get('agent_id', -1)
            spm = detail.get('spm', np.zeros((12, 12)))
            angle = detail.get('velocity_angle', 0.0)
            neighbors = detail.get('neighbor_ids', [])

            stats_text = f"""<b>Agent #{agent_id}</b><br><br>
<b>SPM Data:</b><br>
Shape: {spm.shape[0]}×{spm.shape[1]}<br>
Min: {spm.min():.3f}<br>
Max: {spm.max():.3f}<br><br>
<b>Heading:</b><br>
{np.degrees(angle):.1f}°<br><br>
<b>Neighbors ({len(neighbors)}):</b><br>
{', '.join(f'#{n}' for n in neighbors)}
"""

            self.stats_label.setText(stats_text)

            # Render SPM visualizations
            if spm is not None and spm.shape == (12, 12):
                self._render_heatmap(spm)
                self._render_polar(spm, angle)

        except (KeyError, AttributeError, TypeError) as e:
            logger.error(f"Failed to update agent detail: {e}")
            self.clear()

    def _render_heatmap(self, spm: np.ndarray):
        """Render SPM as 12x12 heatmap

        Args:
            spm: 12x12 numpy array of saliency values
        """
        self.heatmap_ax.clear()

        # Plot heatmap (origin='lower' puts r=0 at bottom)
        im = self.heatmap_ax.imshow(
            spm,
            cmap='viridis',
            aspect='auto',
            origin='lower',
            interpolation='nearest',
            vmin=0.0,
            vmax=1.0
        )

        # Labels
        self.heatmap_ax.set_title("SPM Heatmap (12×12)")
        self.heatmap_ax.set_xlabel("Angular Bin (0°-360°)")
        self.heatmap_ax.set_ylabel("Radial Bin (near → far)")

        # Ticks
        self.heatmap_ax.set_xticks(np.arange(12))
        self.heatmap_ax.set_xticklabels([f"{i*30}°" for i in range(12)])
        self.heatmap_ax.set_yticks(np.arange(12))

        # Colorbar (create once, reuse)
        if self.heatmap_colorbar is None:
            self.heatmap_colorbar = self.heatmap_figure.colorbar(im, ax=self.heatmap_ax)
            self.heatmap_colorbar.set_label('Saliency')

        self.heatmap_figure.tight_layout()
        self.heatmap_canvas.draw()

    def _render_polar(self, spm: np.ndarray, heading_rad: float):
        """Render SPM as polar plot with 270° FOV and ego-centric rotation

        Args:
            spm: 12x12 numpy array of saliency values
            heading_rad: Agent heading in radians
        """
        self.polar_ax.clear()

        # SPM dimensions
        n_radial = spm.shape[0]   # 12
        n_angular = spm.shape[1]  # 12

        # Create polar coordinates
        # Angular bins: 0° to 360° (12 bins, 30° each)
        theta_edges = np.linspace(0, 2*np.pi, n_angular + 1)

        # Radial bins: 0 to max_radius (12 bins)
        r_edges = np.linspace(0, 10.0, n_radial + 1)  # Assume max radius = 10 units

        # Create meshgrid for pcolormesh
        theta_mesh, r_mesh = np.meshgrid(theta_edges, r_edges)

        # Rotate coordinates to make agent heading = 0° (up)
        # Subtract heading to rotate world, not agent
        theta_mesh_rotated = theta_mesh - heading_rad

        # Plot as pcolormesh (filled patches)
        mesh = self.polar_ax.pcolormesh(
            theta_mesh_rotated, r_mesh, spm,
            cmap='viridis',
            vmin=0.0,
            vmax=1.0,
            shading='flat'
        )

        # Add 270° FOV mask (gray out rear 90°: -45° to +45° from rear)
        # Rear direction = heading + π (relative to rotated coordinates)
        rear_start = np.pi - np.pi/4  # 135° (relative to heading=0)
        rear_end = np.pi + np.pi/4    # 225° (relative to heading=0)

        # Draw gray wedge for blind spot
        theta_blind = np.linspace(rear_start, rear_end, 50)
        r_blind = np.full_like(theta_blind, 10.0)
        self.polar_ax.fill_between(
            theta_blind, 0, r_blind,
            color='gray',
            alpha=0.5,
            label='Blind spot (90°)'
        )

        # Configure polar axis
        self.polar_ax.set_theta_zero_location('N')  # 0° at top
        self.polar_ax.set_theta_direction(-1)        # Clockwise
        self.polar_ax.set_rlabel_position(45)
        self.polar_ax.set_title(f"SPM Polar (270° FOV)\nHeading: {np.degrees(heading_rad):.1f}°")

        # Colorbar (create once, reuse)
        if self.polar_colorbar is None:
            self.polar_colorbar = self.polar_figure.colorbar(mesh, ax=self.polar_ax)
            self.polar_colorbar.set_label('Saliency')

        self.polar_figure.tight_layout()
        self.polar_canvas.draw()

    def clear(self):
        """Clear all displays"""
        self.current_detail = None
        self.stats_label.setText("No agent selected")

        # Clear heatmap
        self.heatmap_ax.clear()
        self.heatmap_ax.set_title("SPM Heatmap (12×12)")
        self.heatmap_ax.text(0.5, 0.5, 'No agent selected',
                             ha='center', va='center', transform=self.heatmap_ax.transAxes)
        self.heatmap_figure.tight_layout()
        self.heatmap_canvas.draw()

        # Clear polar plot
        self.polar_ax.clear()
        self.polar_ax.set_title("SPM Polar (270° FOV)")
        self.polar_ax.text(0.5, 0.5, 'No agent selected',
                           ha='center', va='center',
                           transform=self.polar_ax.transAxes)
        self.polar_figure.tight_layout()
        self.polar_canvas.draw()
