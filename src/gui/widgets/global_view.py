"""
Global View Widget - Agent visualization using matplotlib

Displays all agents in 2D torus world with:
- Position (circles)
- Velocity vectors (arrows)
- Haze (color: blue=low, yellow=high)
- Fatigue (size: small=energetic, large=tired)
"""

import logging
from typing import List, Dict, Any, Optional
import numpy as np
from PyQt6.QtWidgets import QWidget, QVBoxLayout
from matplotlib.backends.backend_qtagg import FigureCanvasQTAgg
from matplotlib.figure import Figure
from matplotlib.colors import LinearSegmentedColormap

logger = logging.getLogger(__name__)


class GlobalViewWidget(QWidget):
    """Widget displaying global agent view using matplotlib"""

    def __init__(self, parent=None, world_size=(10.0, 10.0)):
        super().__init__(parent)

        # World size for torus visualization
        self.world_size = world_size

        # Create custom blue→yellow colormap for haze
        self.haze_cmap = LinearSegmentedColormap.from_list('haze', ['blue', 'yellow'])

        # Create matplotlib figure
        self.figure = Figure(figsize=(8, 8))
        self.canvas = FigureCanvasQTAgg(self.figure)
        self.ax = self.figure.add_subplot(111)

        # Setup layout
        layout = QVBoxLayout()
        layout.addWidget(self.canvas)
        layout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(layout)

        # Initialize plot
        self._setup_plot()

        # Data storage
        self.agents_data: List[Dict[str, Any]] = []
        self.selected_agent_id: Optional[int] = None

        # Artist storage for reuse (avoid recreating every frame)
        self.scatter_artist = None
        self.quiver_artist = None
        self.colorbar = None

    def _setup_plot(self):
        """Initialize plot appearance"""
        self.ax.set_xlim(0, self.world_size[0])
        self.ax.set_ylim(0, self.world_size[1])
        self.ax.set_aspect('equal')
        self.ax.set_xlabel('X Position')
        self.ax.set_ylabel('Y Position')
        self.ax.set_title('EPH v2.1 - Global Agent View')
        self.ax.grid(True, alpha=0.3)

    def update_agents(self, agents: List[Dict[str, Any]]):
        """
        Update agent visualization

        Args:
            agents: List of agent dicts with keys:
                    'agent_id', 'x', 'y', 'vx', 'vy',
                    'haze_mean', 'fatigue', 'efe'
        """
        self.agents_data = agents
        self._render()

    def set_world_size(self, width: float, height: float):
        """Update world dimensions for torus visualization"""
        self.world_size = (width, height)
        self._setup_plot()
        self.canvas.draw_idle()

    def _render(self):
        """Render all agents to canvas (optimized)"""
        if not self.agents_data:
            return

        try:
            # Extract data for plotting
            x = np.array([a['x'] for a in self.agents_data])
            y = np.array([a['y'] for a in self.agents_data])
            vx = np.array([a['vx'] for a in self.agents_data])
            vy = np.array([a['vy'] for a in self.agents_data])
            haze = np.array([a['haze_mean'] for a in self.agents_data])
            fatigue = np.array([a['fatigue'] for a in self.agents_data])
        except KeyError as e:
            logger.warning(f"Missing required agent field {e}, skipping render")
            self.canvas.draw_idle()
            return

        colors = haze
        sizes = 50 + 200 * fatigue

        # Update or create scatter plot
        if self.scatter_artist is None:
            self.scatter_artist = self.ax.scatter(
                x, y, c=colors, s=sizes,
                cmap=self.haze_cmap, alpha=0.7,
                edgecolors='black', linewidths=1,
                vmin=0, vmax=1  # Fixed color scale
            )

            # Add colorbar once
            if self.colorbar is None:
                self.colorbar = self.figure.colorbar(
                    self.scatter_artist, ax=self.ax,
                    label='Haze Mean'
                )
        else:
            # Reuse existing scatter artist (update data)
            self.scatter_artist.set_offsets(np.c_[x, y])
            self.scatter_artist.set_array(colors)
            self.scatter_artist.set_sizes(sizes)

        # Update or create quiver plot
        if self.quiver_artist is None:
            self.quiver_artist = self.ax.quiver(
                x, y, vx, vy,
                color='red', alpha=0.6,
                scale=10, width=0.003
            )
        else:
            # Reuse quiver (update positions and vectors)
            self.quiver_artist.set_offsets(np.c_[x, y])
            self.quiver_artist.set_UVC(vx, vy)

        # Redraw canvas
        self.canvas.draw_idle()
