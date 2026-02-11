"""Manual test for GlobalViewWidget"""
import sys
from PyQt6.QtWidgets import QApplication
from widgets.global_view import GlobalViewWidget

def main():
    app = QApplication(sys.argv)

    widget = GlobalViewWidget()
    widget.setWindowTitle("GlobalView Test")
    widget.resize(800, 800)

    # Test data: 5 agents
    test_agents = [
        {'agent_id': 0, 'x': 2.0, 'y': 3.0, 'vx': 0.5, 'vy': 0.3,
         'haze_mean': 0.2, 'fatigue': 0.1, 'efe': 0.0},
        {'agent_id': 1, 'x': 5.0, 'y': 5.0, 'vx': -0.3, 'vy': 0.6,
         'haze_mean': 0.5, 'fatigue': 0.3, 'efe': 0.0},
        {'agent_id': 2, 'x': 8.0, 'y': 2.0, 'vx': 0.1, 'vy': -0.4,
         'haze_mean': 0.8, 'fatigue': 0.6, 'efe': 0.0},
        {'agent_id': 3, 'x': 3.0, 'y': 7.0, 'vx': 0.4, 'vy': 0.2,
         'haze_mean': 0.3, 'fatigue': 0.2, 'efe': 0.0},
        {'agent_id': 4, 'x': 7.0, 'y': 8.0, 'vx': -0.2, 'vy': -0.3,
         'haze_mean': 0.9, 'fatigue': 0.8, 'efe': 0.0},
    ]

    widget.update_agents(test_agents)
    widget.show()

    sys.exit(app.exec())

if __name__ == '__main__':
    main()
