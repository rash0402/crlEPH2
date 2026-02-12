"""
Global View Widget - Agent visualization using PyQtGraph

Displays all agents in 2D torus world with:
- Position (circles)
- Velocity vectors (arrows)
- Haze (color: blue=low, yellow=high)
- Fatigue (size: small=energetic, large=tired)

Performance: 10-100x faster than matplotlib version
"""

import logging
from typing import List, Dict, Any, Optional
import numpy as np
from PyQt6.QtWidgets import QWidget, QVBoxLayout
from PyQt6.QtGui import QColor
from PyQt6.QtCore import pyqtSignal
import pyqtgraph as pg

logger = logging.getLogger(__name__)


class GlobalViewWidget(QWidget):
    """Widget displaying global agent view using PyQtGraph"""

    # Signal emitted when agent is selected
    agent_selected = pyqtSignal(int)  # agent_id

    def __init__(self, parent=None, world_size=(20.0, 20.0)):
        super().__init__(parent)

        # World size for torus visualization
        # Matches C++ constants: WORLD_SIZE=20.0 with range [-10, 10] x [-10, 10]
        # Torus wrapping is implemented in eph_agent.hpp (wrap_position)
        self.world_size = world_size

        # Create PyQtGraph plot widget
        self.plot_widget = pg.PlotWidget()
        self.plot_item = self.plot_widget.getPlotItem()

        # Create colorbar/gradient widget for haze legend
        self.gradient_widget = pg.GradientWidget(orientation='right')
        self.gradient_widget.setMaximumWidth(30)
        # Blue to yellow gradient
        self.gradient_widget.setColorMap(pg.ColorMap(
            pos=[0.0, 1.0],
            color=[(0, 0, 255), (255, 255, 0)]
        ))

        # Setup layout (horizontal: plot + colorbar)
        from PyQt6.QtWidgets import QHBoxLayout
        layout = QHBoxLayout()
        layout.addWidget(self.plot_widget)
        layout.addWidget(self.gradient_widget)
        layout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(layout)

        # Initialize plot
        self._setup_plot()

        # Data storage
        self.agents_data: List[Dict[str, Any]] = []
        self.selected_agent_id: Optional[int] = None

        # Graphics items for reuse (avoid recreating every frame)
        self.scatter_item = None
        self.velocity_item = None
        self.colorbar = None
        self.highlight_item = None

    def _setup_plot(self):
        """Initialize plot appearance"""
        # Center view at origin since agents spawn in [-10, 10]
        half_width = self.world_size[0] / 2.0
        half_height = self.world_size[1] / 2.0

        self.plot_item.setXRange(-half_width, half_width)
        self.plot_item.setYRange(-half_height, half_height)
        self.plot_item.setAspectLocked(True)
        self.plot_item.setLabel('bottom', 'X Position')
        self.plot_item.setLabel('left', 'Y Position')
        self.plot_item.setTitle('EPH v2.1 - Global Agent View (PyQtGraph)')
        self.plot_item.showGrid(x=True, y=True, alpha=0.3)

        # Add origin marker
        origin_marker = pg.ScatterPlotItem(
            pos=[(0, 0)],
            symbol='+',
            size=20,
            pen=pg.mkPen('k', width=2),
            brush=None
        )
        self.plot_item.addItem(origin_marker)

    def update_agents(self, agents: List[Dict[str, Any]]):
        """
        Update agent visualization

        Args:
            agents: List of agent dicts with keys:
                    'agent_id', 'x', 'y', 'vx', 'vy',
                    'haze_mean', 'fatigue', 'efe'
        """
        self.agents_data = agents
        if not agents:
            logger.warning("update_agents called with empty agents list")
            return
        self._render()

    def set_world_size(self, width: float, height: float):
        """Update world dimensions for torus visualization"""
        self.world_size = (width, height)
        self._setup_plot()

    def _render(self):
        """Render all agents to canvas (optimized)"""
        if not self.agents_data:
            return

        try:
            # Extract data for plotting
            n_agents = len(self.agents_data)
            positions = np.zeros((n_agents, 2))
            haze = np.zeros(n_agents)
            fatigue = np.zeros(n_agents)
            velocities = np.zeros((n_agents, 2))

            for i, agent in enumerate(self.agents_data):
                positions[i] = [agent['x'], agent['y']]
                velocities[i] = [agent['vx'], agent['vy']]
                haze[i] = agent['haze_mean']
                fatigue[i] = agent['fatigue']

        except KeyError as e:
            logger.warning(f"Missing required agent field {e}, skipping render")
            return

        # Map haze to colors (blue=0 to yellow=1)
        colors = self._haze_to_colors(haze)

        # Map fatigue to sizes (50-250 range)
        sizes = 10 + 20 * fatigue

        # Update or create scatter plot
        if self.scatter_item is None:
            self.scatter_item = pg.ScatterPlotItem(
                pos=positions,
                brush=colors,
                size=sizes,
                pen=pg.mkPen('k', width=1),
                symbol='o'
            )
            self.plot_item.addItem(self.scatter_item)
        else:
            # Update existing scatter plot
            self.scatter_item.setData(
                pos=positions,
                brush=colors,
                size=sizes
            )

        # Highlight selected agent with yellow border
        if self.selected_agent_id is not None:
            for i, agent in enumerate(self.agents_data):
                if agent['agent_id'] == self.selected_agent_id:
                    # Create highlight scatter item
                    if self.highlight_item is None:
                        self.highlight_item = pg.ScatterPlotItem(
                            pen=pg.mkPen('y', width=3),
                            brush=None,
                            size=sizes[i] + 10,
                            symbol='o'
                        )
                        self.plot_item.addItem(self.highlight_item)
                    else:
                        self.highlight_item.setData(
                            pos=[positions[i]],
                            size=[sizes[i] + 10]
                        )
                    break
        elif self.highlight_item is not None:
            # Remove highlight if no agent is selected
            self.plot_item.removeItem(self.highlight_item)
            self.highlight_item = None

        # Update or create velocity vectors
        # Create line segments for velocity arrows
        velocity_lines = self._create_velocity_lines(positions, velocities)

        if self.velocity_item is None:
            self.velocity_item = pg.PlotDataItem(
                velocity_lines[:, 0],
                velocity_lines[:, 1],
                pen=pg.mkPen('r', width=2),
                connect='pairs'  # Connect points in pairs (line segments)
            )
            self.plot_item.addItem(self.velocity_item)
        else:
            # Update existing velocity vectors
            self.velocity_item.setData(
                velocity_lines[:, 0],
                velocity_lines[:, 1],
                connect='pairs'
            )

    def mousePressEvent(self, event):
        """Handle mouse click to select agent"""
        from PyQt6.QtCore import Qt
        if event.button() != Qt.MouseButton.LeftButton:
            return

        # Get click position in plot coordinates
        mouse_point = self.plot_item.vb.mapSceneToView(event.pos())
        click_x = mouse_point.x()
        click_y = mouse_point.y()

        # Find nearest agent within selection radius
        min_dist = float('inf')
        selected_id = None
        selection_radius = 1.0  # 1 unit in world coordinates

        for agent in self.agents_data:
            dx = agent['x'] - click_x
            dy = agent['y'] - click_y
            dist = (dx**2 + dy**2)**0.5

            if dist < selection_radius and dist < min_dist:
                min_dist = dist
                selected_id = agent['agent_id']

        if selected_id is not None:
            self.selected_agent_id = selected_id
            self.agent_selected.emit(selected_id)
            self._render()  # Re-render to show selection

    def _haze_to_colors(self, haze: np.ndarray) -> List:
        """
        Convert haze values [0, 1] to blue→yellow gradient colors

        Args:
            haze: Array of haze values [0, 1]

        Returns:
            List of QColor objects
        """
        colors = []
        for h in haze:
            # Clamp to [0, 1]
            h = np.clip(h, 0.0, 1.0)

            # Blue (0, 0, 255) → Yellow (255, 255, 0)
            r = int(h * 255)
            g = int(h * 255)
            b = int((1.0 - h) * 255)

            colors.append(pg.mkBrush(QColor(r, g, b, 178)))  # alpha=0.7

        return colors

    def _create_velocity_lines(self, positions: np.ndarray, velocities: np.ndarray) -> np.ndarray:
        """
        Create line segments for velocity vectors with boundary clipping

        Args:
            positions: (N, 2) array of agent positions
            velocities: (N, 2) array of agent velocities

        Returns:
            (2*N, 2) array of line segment endpoints (pairs of points)
        """
        n_agents = len(positions)

        # Scale factor for velocity visualization
        scale = 0.5

        # Create endpoints for each velocity vector
        endpoints = positions + velocities * scale

        # Clip endpoints at world boundaries instead of wrapping
        # This prevents arrows from extending across the entire world
        world_min = -self.world_size[0] / 2.0
        world_max = self.world_size[0] / 2.0

        # Clip x and y coordinates to world boundaries
        endpoints[:, 0] = np.clip(endpoints[:, 0], world_min, world_max)
        endpoints[:, 1] = np.clip(endpoints[:, 1], world_min, world_max)

        # Interleave start and end points: [start0, end0, start1, end1, ...]
        lines = np.empty((n_agents * 2, 2))
        lines[0::2] = positions  # Start points at even indices
        lines[1::2] = endpoints  # End points at odd indices

        return lines
