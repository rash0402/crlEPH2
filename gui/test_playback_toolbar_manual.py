"""Manual test for PlaybackToolbar"""
import sys
from PyQt6.QtWidgets import QApplication, QMainWindow
from widgets.playback_toolbar import PlaybackToolbar


def main():
    app = QApplication(sys.argv)

    window = QMainWindow()
    toolbar = PlaybackToolbar()

    toolbar.play_clicked.connect(lambda: print("Play"))
    toolbar.pause_clicked.connect(lambda: print("Pause"))
    toolbar.stop_clicked.connect(lambda: print("Stop"))
    toolbar.speed_changed.connect(lambda s: print(f"Speed: {s}"))

    window.addToolBar(toolbar)
    window.setWindowTitle("PlaybackToolbar Test")
    window.resize(600, 400)
    window.show()

    sys.exit(app.exec())


if __name__ == '__main__':
    main()
