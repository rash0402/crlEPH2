"""
Global View Widget - Agent visualization using matplotlib

Displays all agents in 2D torus world with:
- Position (circles)
- Velocity vectors (arrows)
- Haze (color: blue=low, yellow=high)
- Fatigue (size: small=energetic, large=tired)
"""

from typing import List, Dict, Any, Optional
import numpy as np
from PyQt6.QtWidgets import QWidget, QVBoxLayout
from matplotlib.backends.backend_qtagg import FigureCanvasQTAgg
from matplotlib.figure import Figure
from matplotlib.colors import LinearSegmentedColormap


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
        """Render all agents to canvas"""
        self.ax.clear()
        self._setup_plot()

        if not self.agents_data:
            self.canvas.draw_idle()
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
            print(f"Warning: Missing required agent field {e}, skipping render")
            self.canvas.draw_idle()
            return

        # Normalize haze for colormap (0=blue, 1=yellow)
        # Assume haze range [0, 1], adjust if needed
        colors = haze

        # Map fatigue to marker size (small=0, large=1)
        # Base size 50, scale by fatigue
        sizes = 50 + 200 * fatigue

        # Plot agents as scatter
        self.ax.scatter(x, y, c=colors, s=sizes,
                       cmap=self.haze_cmap, alpha=0.7,
                       edgecolors='black', linewidths=1)

        # Plot velocity vectors
        self.ax.quiver(x, y, vx, vy,
                      color='red', alpha=0.6,
                      scale=10, width=0.003)

        # Add colorbar for haze
        # TODO: Persistent colorbar (reuse across renders)

        self.canvas.draw_idle()
