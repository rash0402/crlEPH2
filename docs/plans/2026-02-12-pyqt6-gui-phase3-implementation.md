# EPH v2.1 PyQt6 GUI - Phase 3 Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Implement agent selection and SPM (Saliency Polar Map) detail view with 12×12 polar coordinate visualization

**Architecture:** Add click-to-select agent interaction in global view, extend C++ UDP protocol to send selected agent's SPM data, create 3-column detail panel (heatmap/polar/stats) in bottom dock. PyQtGraph for performance, matplotlib for polar plots.

**Tech Stack:** PyQt6 6.4+, PyQtGraph 0.13+, matplotlib 3.8+, numpy 1.24+, C++17 with existing EPH v2.1 engine

---

## Task 1: Agent Selection in Global View

**Files:**
- Modify: `src/gui/widgets/global_view.py:23-244`
- Modify: `src/gui/widgets/main_window.py:16-130`

**Step 1: Add click signal to GlobalViewWidget**

In `src/gui/widgets/global_view.py`, add after line 16:

```python
from PyQt6.QtCore import pyqtSignal
```

Add signal declaration after line 60 (after `self.selected_agent_id: Optional[int] = None`):

```python
# Signal emitted when agent is selected
agent_selected = pyqtSignal(int)  # agent_id
```

**Step 2: Implement click handler**

Add method after `_render()` (around line 177):

```python
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
```

**Step 3: Highlight selected agent in render**

Modify `_render()` method around line 140-156 to add highlight:

After line 156 (`self.scatter_item.setData(...)`), add:

```python
# Highlight selected agent with yellow border
if self.selected_agent_id is not None:
    for i, agent in enumerate(self.agents_data):
        if agent['agent_id'] == self.selected_agent_id:
            # Create highlight scatter item
            if not hasattr(self, 'highlight_item') or self.highlight_item is None:
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
```

**Step 4: Connect signal in MainWindow**

In `src/gui/widgets/main_window.py`, modify `__init__` around line 46:

After line 46 (`self.setCentralWidget(self.global_view)`), add:

```python
# Connect agent selection signal
self.global_view.agent_selected.connect(self._on_agent_selected)
```

Add handler method before `_create_parameter_dock()` (around line 72):

```python
def _on_agent_selected(self, agent_id: int):
    """Handle agent selection from global view"""
    logger.info(f"Agent {agent_id} selected")
    # TODO: Update detail panel (Task 3)
    # TODO: Send select_agent command to C++ (Task 2)
```

**Step 5: Test manually**

Run: `./run.sh`
Expected:
- Click on agent → console log "Agent X selected"
- Selected agent shows yellow border

**Step 6: Commit**

```bash
git add src/gui/widgets/global_view.py src/gui/widgets/main_window.py
git commit -m "feat: Add agent selection with click interaction

- Add mousePressEvent handler to detect clicks
- Highlight selected agent with yellow border
- Emit agent_selected signal with agent_id
- Connect signal in MainWindow"
```

---

## Task 2: C++ Protocol Extension for Agent Details

**Files:**
- Modify: `src/cpp_server/protocol.hpp:1-138`
- Modify: `src/cpp_server/main.cpp:1-180`
- Modify: `src/gui/bridge/protocol.py:1-151`

**Step 1: Define AgentDetailData struct in C++**

In `src/cpp_server/protocol.hpp`, add after `MetricsData` (around line 68):

```cpp
/**
 * @brief Agent detail data (608 bytes)
 *
 * Contains SPM (12x12) and neighbor information for selected agent
 */
struct AgentDetailData {
    uint16_t agent_id;       // Selected agent ID
    uint16_t num_neighbors;  // Number of valid neighbors
    float spm_data[144];     // SPM 12x12 = 144 floats
    float velocity_angle;    // Agent heading in radians
    uint16_t neighbor_ids[6];// Neighbor agent IDs
    uint16_t padding[3];     // Alignment to 8-byte boundary
} __attribute__((packed));

static_assert(sizeof(AgentDetailData) == 608, "AgentDetailData must be 608 bytes");
```

**Step 2: Add selected_agent_id state in main.cpp**

In `src/cpp_server/main.cpp`, add after line 50 (after `int sleep_ms`):

```cpp
// Agent selection state
std::optional<size_t> selected_agent_id;
```

**Step 3: Handle select_agent command**

In `src/cpp_server/main.cpp`, add after "set_parameters" handler (around line 84):

```cpp
else if (cmd_type == "select_agent") {
    int agent_id = command.value().value("agent_id", -1);
    if (agent_id >= 0 && agent_id < static_cast<int>(N_AGENTS)) {
        selected_agent_id = static_cast<size_t>(agent_id);
        std::cout << "  Selected agent: " << agent_id << std::endl;
    } else {
        selected_agent_id = std::nullopt;
        std::cout << "  Deselected agent (invalid ID)" << std::endl;
    }
}
```

**Step 4: Send agent detail packet**

In `src/cpp_server/main.cpp`, add after sending state packet (around line 150):

```cpp
// Send selected agent detail (if any)
if (selected_agent_id.has_value() && timestep % SEND_INTERVAL == 0) {
    const auto& agent = swarm.get_agent(selected_agent_id.value());
    const auto& agent_state = agent.state();
    const auto& spm_data = agent.haze();  // 12x12 Matrix12x12

    udp::AgentDetailData detail;
    detail.agent_id = static_cast<uint16_t>(selected_agent_id.value());

    // Flatten SPM data (row-major: r=0..11, theta=0..11)
    for (int r = 0; r < 12; ++r) {
        for (int theta = 0; theta < 12; ++theta) {
            detail.spm_data[r * 12 + theta] = static_cast<float>(spm_data(r, theta));
        }
    }

    // Velocity angle (atan2 returns [-π, π])
    detail.velocity_angle = static_cast<float>(
        std::atan2(agent_state.velocity.y(), agent_state.velocity.x())
    );

    // Get neighbors
    auto neighbors = swarm.find_neighbors(selected_agent_id.value());
    detail.num_neighbors = static_cast<uint16_t>(std::min(neighbors.size(), size_t(6)));
    for (size_t i = 0; i < detail.num_neighbors; ++i) {
        detail.neighbor_ids[i] = static_cast<uint16_t>(neighbors[i]);
    }
    for (size_t i = detail.num_neighbors; i < 6; ++i) {
        detail.neighbor_ids[i] = 0xFFFF;  // Invalid marker
    }

    // Send via UDP (reuse send socket, different format)
    // For simplicity, append to state packet (alternative: separate port)
    // TODO: Implement proper serialization
}
```

**Step 5: Python protocol deserialization**

In `src/gui/bridge/protocol.py`, add after `MetricsData` (around line 88):

```python
class AgentDetailData:
    """Agent detail data (608 bytes)"""
    SIZE = 608
    # uint16 agent_id, uint16 num_neighbors, 144 floats, float angle, 6 uint16, 3 uint16 padding
    FORMAT = '<HH144ff6H3H'

    @staticmethod
    def parse(data: bytes) -> Optional[Dict[str, Any]]:
        if len(data) < AgentDetailData.SIZE:
            return None

        values = struct.unpack(AgentDetailData.FORMAT, data[:AgentDetailData.SIZE])

        agent_id = values[0]
        num_neighbors = values[1]
        spm_flat = values[2:146]  # 144 floats
        velocity_angle = values[146]
        neighbor_ids_raw = values[147:153]  # 6 uint16

        # Reshape SPM to 12x12
        import numpy as np
        spm_matrix = np.array(spm_flat).reshape((12, 12))

        # Filter valid neighbor IDs
        neighbor_ids = [nid for nid in neighbor_ids_raw if nid != 0xFFFF][:num_neighbors]

        return {
            'agent_id': agent_id,
            'spm': spm_matrix,
            'velocity_angle': velocity_angle,
            'neighbor_ids': neighbor_ids
        }
```

**Step 6: Test protocol (manual verification)**

Add temporary print in `src/gui/main.py` to verify:

```python
# After packet deserialization
if 'agent_detail' in packet:
    detail = packet['agent_detail']
    print(f"Agent detail: ID={detail['agent_id']}, SPM shape={detail['spm'].shape}")
```

**Step 7: Commit**

```bash
git add src/cpp_server/protocol.hpp src/cpp_server/main.cpp src/gui/bridge/protocol.py
git commit -m "feat: Add agent detail protocol with SPM data

- Define AgentDetailData struct (608 bytes)
- Handle select_agent command in C++
- Serialize and send selected agent's SPM (12x12)
- Add Python deserialization for agent details"
```

---

## Task 3: Agent Detail Panel Widget (Layout)

**Files:**
- Create: `src/gui/widgets/agent_detail_panel.py`
- Modify: `src/gui/widgets/main_window.py:16-130`

**Step 1: Create AgentDetailPanel skeleton**

Create `src/gui/widgets/agent_detail_panel.py`:

```python
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
```

**Step 2: Add detail panel to MainWindow**

In `src/gui/widgets/main_window.py`, add import after line 13:

```python
from .agent_detail_panel import AgentDetailPanel
```

Add method after `_create_parameter_dock()` (around line 82):

```python
def _create_detail_dock(self):
    """Create agent detail panel dock widget"""
    self.detail_panel = AgentDetailPanel()

    dock = QDockWidget("Agent Detail", self)
    dock.setWidget(self.detail_panel)
    dock.setAllowedAreas(Qt.DockWidgetArea.BottomDockWidgetArea)

    self.addDockWidget(Qt.DockWidgetArea.BottomDockWidgetArea, dock)
```

Call it in `__init__` after parameter dock creation (around line 82):

```python
# Bottom dock: Agent Detail Panel
self._create_detail_dock()
```

**Step 3: Connect agent selection to detail panel**

Modify `_on_agent_selected()` in `main_window.py`:

```python
def _on_agent_selected(self, agent_id: int):
    """Handle agent selection from global view"""
    logger.info(f"Agent {agent_id} selected")
    # Detail panel will update when detail packet arrives
    # For now, clear and show placeholder
    self.detail_panel.clear()
```

**Step 4: Test layout**

Run: `./run.sh`
Expected:
- Bottom dock appears with 3 columns
- Click agent → "No agent selected" message (detail data not wired yet)

**Step 5: Commit**

```bash
git add src/gui/widgets/agent_detail_panel.py src/gui/widgets/main_window.py
git commit -m "feat: Add agent detail panel layout (3 columns)

- Create AgentDetailPanel with heatmap/polar/stats columns
- Add bottom dock to MainWindow
- Wire up agent selection handler
- Placeholder labels for SPM visualizations"
```

---

## Task 4: SPM Heatmap Visualization

**Files:**
- Modify: `src/gui/widgets/agent_detail_panel.py:1-80`

**Step 1: Add matplotlib canvas for heatmap**

In `agent_detail_panel.py`, add imports:

```python
import matplotlib
matplotlib.use('Qt5Agg')  # Must be before importing pyplot
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg
from matplotlib.figure import Figure
```

**Step 2: Create heatmap canvas in __init__**

Replace `self.heatmap_widget = QLabel(...)` with:

```python
# Column 1: SPM Heatmap (matplotlib)
self.heatmap_figure = Figure(figsize=(4, 4), dpi=80)
self.heatmap_canvas = FigureCanvasQTAgg(self.heatmap_figure)
self.heatmap_ax = self.heatmap_figure.add_subplot(111)
self.heatmap_ax.set_title("SPM Heatmap (12×12)")
self.heatmap_ax.set_xlabel("Angular Bin (θ)")
self.heatmap_ax.set_ylabel("Radial Bin (r)")
self.heatmap_figure.tight_layout()
main_layout.addWidget(self.heatmap_canvas)
```

**Step 3: Implement heatmap rendering**

Add method after `update_detail()`:

```python
def _render_heatmap(self, spm: np.ndarray):
    """Render SPM as 12x12 heatmap"""
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

    # Colorbar
    if not hasattr(self, 'heatmap_colorbar') or self.heatmap_colorbar is None:
        self.heatmap_colorbar = self.heatmap_figure.colorbar(im, ax=self.heatmap_ax)
        self.heatmap_colorbar.set_label('Saliency')

    self.heatmap_figure.tight_layout()
    self.heatmap_canvas.draw()
```

**Step 4: Call render in update_detail**

Modify `update_detail()` to call rendering:

```python
def update_detail(self, detail: Dict[str, Any]):
    """Update panel with agent detail data"""
    self.current_detail = detail

    # ... existing stats update code ...

    # Render SPM heatmap
    spm = detail.get('spm', None)
    if spm is not None:
        self._render_heatmap(spm)
```

**Step 5: Test heatmap (with mock data)**

Add temporary test in `main_window.py`:

```python
# In _on_agent_selected, add:
mock_spm = np.random.rand(12, 12)
self.detail_panel.update_detail({
    'agent_id': agent_id,
    'spm': mock_spm,
    'velocity_angle': 0.0,
    'neighbor_ids': []
})
```

Run: `./run.sh`
Expected: Click agent → 12×12 heatmap displays with viridis colormap

Remove mock data after testing.

**Step 6: Commit**

```bash
git add src/gui/widgets/agent_detail_panel.py
git commit -m "feat: Add SPM heatmap visualization (12×12 matrix)

- Replace QLabel with matplotlib FigureCanvas
- Implement _render_heatmap with viridis colormap
- Add colorbar and axis labels
- Display angular/radial bins"
```

---

## Task 5: SPM Polar Visualization (270° FOV)

**Files:**
- Modify: `src/gui/widgets/agent_detail_panel.py:1-120`

**Step 1: Create polar canvas in __init__**

Replace `self.polar_widget = QLabel(...)` with:

```python
# Column 2: SPM Polar (matplotlib polar plot)
self.polar_figure = Figure(figsize=(4, 4), dpi=80)
self.polar_canvas = FigureCanvasQTAgg(self.polar_figure)
self.polar_ax = self.polar_figure.add_subplot(111, projection='polar')
self.polar_ax.set_title("SPM Polar (270° FOV)")
self.polar_figure.tight_layout()
main_layout.addWidget(self.polar_canvas)
```

**Step 2: Implement polar rendering with 270° mask**

Add method after `_render_heatmap()`:

```python
def _render_polar(self, spm: np.ndarray, heading_rad: float):
    """Render SPM as polar plot with 270° FOV and ego-centric rotation"""
    self.polar_ax.clear()

    # SPM dimensions
    n_radial = spm.shape[0]   # 12
    n_angular = spm.shape[1]  # 12

    # Create polar coordinates
    # Angular bins: 0° to 360° (12 bins, 30° each)
    theta_bins = np.linspace(0, 2*np.pi, n_angular, endpoint=False)
    theta_edges = np.linspace(0, 2*np.pi, n_angular + 1)

    # Radial bins: 0 to max_radius (12 bins)
    r_bins = np.linspace(0, 10.0, n_radial)  # Assume max radius = 10 units
    r_edges = np.linspace(0, 10.0, n_radial + 1)

    # Rotate coordinates to make agent heading = 0° (up)
    # Subtract heading to rotate world, not agent
    theta_rotated = theta_bins - heading_rad

    # Plot as pcolormesh (filled patches)
    theta_mesh, r_mesh = np.meshgrid(theta_edges, r_edges)
    theta_mesh_rotated = theta_mesh - heading_rad

    mesh = self.polar_ax.pcolormesh(
        theta_mesh_rotated, r_mesh, spm,
        cmap='viridis',
        vmin=0.0,
        vmax=1.0,
        shading='flat'
    )

    # Add 270° FOV mask (gray out rear 90°: -45° to +45° from rear)
    # Rear direction = heading + π
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

    # Colorbar
    if not hasattr(self, 'polar_colorbar') or self.polar_colorbar is None:
        self.polar_colorbar = self.polar_figure.colorbar(mesh, ax=self.polar_ax)
        self.polar_colorbar.set_label('Saliency')

    self.polar_figure.tight_layout()
    self.polar_canvas.draw()
```

**Step 3: Call polar render in update_detail**

Add to `update_detail()`:

```python
# Render SPM polar plot
if spm is not None:
    self._render_heatmap(spm)
    self._render_polar(spm, detail.get('velocity_angle', 0.0))
```

**Step 4: Test polar plot**

Run: `./run.sh`
Expected:
- Click agent → circular polar plot appears
- 270° colored region, 90° gray blind spot
- Agent heading at top (0°)

**Step 5: Commit**

```bash
git add src/gui/widgets/agent_detail_panel.py
git commit -m "feat: Add SPM polar visualization with 270° FOV

- Create matplotlib polar subplot
- Rotate coordinates to make heading=0° (ego-centric)
- Add gray wedge for 90° blind spot (rear)
- Use pcolormesh for filled polar patches"
```

---

## Task 6: Wire C++ Agent Detail to GUI

**Files:**
- Modify: `src/gui/bridge/udp_client.py:1-90`
- Modify: `src/gui/main.py:40-135`

**Step 1: Add send_select_agent to UDPClient**

In `src/gui/bridge/udp_client.py`, add method after `send_command()`:

```python
def send_select_agent(self, agent_id: int):
    """Send agent selection command to C++"""
    self.send_command({'type': 'select_agent', 'agent_id': agent_id})
```

**Step 2: Handle agent detail packets in receive loop**

This requires modifying C++ to actually send the detail packet. For now, create mock handler in `main.py`.

In `src/gui/main.py`, modify `_on_data_received()` (around line 73):

```python
def _on_data_received(self, packet: Dict[str, Any]):
    """Handle incoming state packet"""
    if packet is None:
        return

    # ... existing update code ...

    # Check if agent detail is present (when implemented in C++)
    if 'agent_detail' in packet:
        detail = packet['agent_detail']
        self.window.detail_panel.update_detail(detail)
```

**Step 3: Send select command when agent clicked**

In `src/gui/widgets/main_window.py`, modify `_on_agent_selected()`:

```python
def _on_agent_selected(self, agent_id: int):
    """Handle agent selection from global view"""
    logger.info(f"Agent {agent_id} selected")

    # Send selection command to C++
    # Note: Access to client via parent application
    from ..main import EPHApplication
    if hasattr(self, 'parent_app'):
        self.parent_app.client.send_select_agent(agent_id)
```

Pass app reference in `main.py` `__init__`:

```python
# In EPHApplication.__init__, after creating window:
self.window.parent_app = self
```

**Step 4: Test end-to-end (with C++ implementation)**

Build C++:
```bash
cd build && cmake --build . --target eph_gui_server && cd ..
```

Run: `./run.sh`
Expected:
- Click agent → C++ logs "Selected agent: X"
- (Agent detail display requires completing C++ serialization from Task 2)

**Step 5: Commit**

```bash
git add src/gui/bridge/udp_client.py src/gui/main.py src/gui/widgets/main_window.py
git commit -m "feat: Wire agent selection to C++ command

- Add send_select_agent to UDPClient
- Send selection command on agent click
- Handle agent_detail packets (placeholder)
- Pass app reference to MainWindow for client access"
```

---

## Task 7: Complete C++ Agent Detail Serialization

**Files:**
- Modify: `src/cpp_server/main.cpp:85-180`
- Modify: `src/cpp_server/udp_server.cpp:102-119`
- Modify: `src/cpp_server/udp_server.hpp:30-50`

**Step 1: Add send_detail method to UDPServer**

In `src/cpp_server/udp_server.hpp`, add declaration after `send_state()`:

```cpp
bool send_detail(const udp::AgentDetailData& detail);
```

In `src/cpp_server/udp_server.cpp`, implement after `send_state()`:

```cpp
bool UDPServer::send_detail(const udp::AgentDetailData& detail) {
    if (!initialized_) {
        return false;
    }

    // Serialize detail data directly (simple binary copy)
    std::vector<uint8_t> buffer(sizeof(detail));
    std::memcpy(buffer.data(), &detail, sizeof(detail));

    ssize_t sent = sendto(send_socket_, buffer.data(), buffer.size(), 0,
                          (struct sockaddr*)&send_addr_, sizeof(send_addr_));

    if (sent < 0) {
        int saved_errno = errno;
        last_error_ = "Failed to send detail: " + std::string(strerror(saved_errno));
        return false;
    }

    return true;
}
```

**Step 2: Complete agent detail sending in main.cpp**

Replace the TODO block from Task 2 Step 4 with:

```cpp
// Send selected agent detail (if any)
if (selected_agent_id.has_value() && timestep % SEND_INTERVAL == 0) {
    const auto& agent = swarm.get_agent(selected_agent_id.value());
    const auto& agent_state = agent.state();
    const auto& spm_matrix = agent.haze();  // 12x12 Matrix12x12

    udp::AgentDetailData detail;
    detail.agent_id = static_cast<uint16_t>(selected_agent_id.value());

    // Flatten SPM data (row-major: r=0..11, theta=0..11)
    for (int r = 0; r < 12; ++r) {
        for (int theta = 0; theta < 12; ++theta) {
            detail.spm_data[r * 12 + theta] = static_cast<float>(spm_matrix(r, theta));
        }
    }

    // Velocity angle (atan2 returns [-π, π])
    detail.velocity_angle = static_cast<float>(
        std::atan2(agent_state.velocity.y(), agent_state.velocity.x())
    );

    // Get neighbors
    auto neighbors = swarm.find_neighbors(selected_agent_id.value());
    detail.num_neighbors = static_cast<uint16_t>(std::min(neighbors.size(), size_t(6)));
    for (size_t i = 0; i < detail.num_neighbors; ++i) {
        detail.neighbor_ids[i] = static_cast<uint16_t>(neighbors[i]);
    }
    for (size_t i = detail.num_neighbors; i < 6; ++i) {
        detail.neighbor_ids[i] = 0xFFFF;  // Invalid marker
    }

    // Send detail packet
    if (!server.send_detail(detail)) {
        std::cerr << "Failed to send agent detail: " << server.get_last_error() << std::endl;
    }
}
```

**Step 3: Update Python to receive detail packets**

In `src/gui/bridge/protocol.py`, modify `deserialize_state_packet()` to handle combined packets:

Note: For simplicity, send detail as separate UDP packet. Python receives both.

In `src/gui/bridge/udp_client.py`, add separate handler:

```python
def receive_detail(self) -> Optional[Dict[str, Any]]:
    """Receive agent detail packet (blocking with timeout)"""
    try:
        data, _ = self.sock.recvfrom(1024)

        # Check if it's detail packet (size == 608)
        if len(data) == 608:
            from .protocol import AgentDetailData
            return AgentDetailData.parse(data)

        return None
    except socket.timeout:
        return None
    except Exception as e:
        logger.error(f"Detail receive error: {e}")
        return None
```

**Step 4: Call receive_detail in main loop**

In `src/gui/main.py`, add after receiving state packet:

```python
# Receive agent detail (if any)
detail = self.client.receive_detail()
if detail:
    self.window.detail_panel.update_detail(detail)
```

**Step 5: Test full pipeline**

Build and run:
```bash
cd build && cmake --build . --target eph_gui_server && cd ..
./run.sh
```

Expected:
- Click agent → SPM heatmap and polar plot display
- Statistics show agent ID, heading, neighbors

**Step 6: Commit**

```bash
git add src/cpp_server/udp_server.{hpp,cpp} src/cpp_server/main.cpp src/gui/bridge/udp_client.py src/gui/main.py
git commit -m "feat: Complete C++ agent detail serialization

- Add send_detail method to UDPServer
- Serialize and send AgentDetailData (608 bytes)
- Add receive_detail to Python UDPClient
- Wire detail packets to GUI update"
```

---

## Task 8: Performance Optimization and Polish

**Files:**
- Modify: `src/gui/widgets/agent_detail_panel.py:1-150`

**Step 1: Add FPS limiting to avoid excessive redraws**

In `agent_detail_panel.py`, add throttling:

```python
import time

class AgentDetailPanel(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)

        # ... existing code ...

        # Throttling
        self.last_update_time = 0.0
        self.min_update_interval = 0.05  # 20 Hz max
```

Modify `update_detail()`:

```python
def update_detail(self, detail: Dict[str, Any]):
    """Update panel with agent detail data (throttled)"""
    # Throttle updates to avoid excessive rendering
    now = time.time()
    if now - self.last_update_time < self.min_update_interval:
        return
    self.last_update_time = now

    # ... rest of existing code ...
```

**Step 2: Cache matplotlib objects to avoid recreation**

Modify `_render_heatmap()` to reuse imshow:

```python
def _render_heatmap(self, spm: np.ndarray):
    """Render SPM as 12x12 heatmap (optimized)"""
    if not hasattr(self, 'heatmap_im') or self.heatmap_im is None:
        # First render - create objects
        self.heatmap_ax.clear()
        self.heatmap_im = self.heatmap_ax.imshow(
            spm, cmap='viridis', aspect='auto',
            origin='lower', interpolation='nearest',
            vmin=0.0, vmax=1.0
        )
        # ... labels, colorbar ...
    else:
        # Subsequent renders - just update data
        self.heatmap_im.set_data(spm)

    self.heatmap_canvas.draw_idle()  # Use draw_idle instead of draw
```

**Step 3: Add clear method for deselection**

Modify `clear()` to properly clear canvases:

```python
def clear(self):
    """Clear all displays"""
    self.current_detail = None
    self.stats_label.setText("No agent selected")

    # Clear matplotlib canvases
    self.heatmap_ax.clear()
    self.heatmap_ax.text(0.5, 0.5, 'No agent selected',
                        ha='center', va='center', transform=self.heatmap_ax.transAxes)
    self.heatmap_canvas.draw_idle()

    self.polar_ax.clear()
    self.polar_ax.text(0, 0, 'No agent selected',
                      ha='center', va='center')
    self.polar_canvas.draw_idle()

    # Reset cached objects
    self.heatmap_im = None
    self.polar_colorbar = None
```

**Step 4: Test performance**

Run: `./run.sh`
Check FPS remains >30 even with detail panel open and updating.

**Step 5: Commit**

```bash
git add src/gui/widgets/agent_detail_panel.py
git commit -m "perf: Optimize agent detail panel rendering

- Throttle updates to 20 Hz max
- Cache matplotlib objects (imshow, colorbar)
- Use draw_idle for non-blocking canvas updates
- Properly clear canvases on deselection"
```

---

## Summary

**Implemented:**
1. ✅ Agent selection with click interaction
2. ✅ C++ protocol extension (AgentDetailData)
3. ✅ 3-column detail panel layout
4. ✅ SPM heatmap visualization (12×12)
5. ✅ SPM polar visualization (270° FOV)
6. ✅ Full data pipeline (C++ → Python → GUI)
7. ✅ Performance optimization

**Testing:**
- Run `./run.sh`
- Click any agent in global view
- Verify:
  - Yellow highlight appears on selected agent
  - Bottom panel shows SPM heatmap and polar plot
  - Statistics display agent info and neighbors
  - FPS remains >30

**Next Steps (Optional):**
- Add metrics panel with time-series plots (φ, χ, haze)
- Add neighbor lines in global view
- Add SPM animation (temporal evolution)
