"""Manual test for ParameterPanel"""
import sys
from PyQt6.QtWidgets import QApplication
from widgets.parameter_panel import ParameterPanel

def on_params_changed(params):
    print(f"Parameters changed: {params}")

def main():
    app = QApplication(sys.argv)

    panel = ParameterPanel()
    panel.parameters_changed.connect(on_params_changed)
    panel.setWindowTitle("ParameterPanel Test")
    panel.resize(400, 400)
    panel.show()

    sys.exit(app.exec())

if __name__ == '__main__':
    main()
