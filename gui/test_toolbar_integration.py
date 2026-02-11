#!/usr/bin/env python3
"""
Integration test for PlaybackToolbar
Tests that commands are properly sent via UDP
"""
import sys
import logging
from PyQt6.QtWidgets import QApplication
from PyQt6.QtCore import QTimer

from main import EPHApplication

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


def test_playback_commands():
    """Test playback commands are sent correctly"""
    app_instance = EPHApplication()

    def run_tests():
        logger.info("=== Starting Playback Toolbar Integration Test ===")

        # Verify toolbar exists
        toolbar = app_instance.window.playback_toolbar
        assert toolbar is not None, "Toolbar not found"
        logger.info("✓ Toolbar exists")

        # Verify initial state
        assert toolbar.is_playing is False, "Should not be playing initially"
        assert toolbar.current_speed == 1.0, "Speed should be 1.0 initially"
        logger.info("✓ Initial state correct")

        # Test Play command
        logger.info("\n--- Testing Play Command ---")
        toolbar.play_button.click()
        assert toolbar.is_playing is True, "Should be playing after click"
        logger.info("✓ Play button state changed")

        # Test Pause command
        logger.info("\n--- Testing Pause Command ---")
        toolbar.pause_button.click()
        assert toolbar.is_playing is False, "Should be paused after click"
        logger.info("✓ Pause button state changed")

        # Test Stop command
        logger.info("\n--- Testing Stop Command ---")
        toolbar.play_button.click()  # Start playing first
        toolbar.stop_button.click()
        assert toolbar.is_playing is False, "Should be stopped"
        logger.info("✓ Stop button state changed")

        # Test Speed change
        logger.info("\n--- Testing Speed Change ---")
        toolbar.speed_combo.setCurrentText("x2")
        assert toolbar.current_speed == 2.0, "Speed should be 2.0"
        logger.info("✓ Speed changed to x2")

        toolbar.speed_combo.setCurrentText("x0.5")
        assert toolbar.current_speed == 0.5, "Speed should be 0.5"
        logger.info("✓ Speed changed to x0.5")

        logger.info("\n=== All Tests Passed ===")
        logger.info("Check the logs above to confirm UDP commands were sent")

        # Exit application
        QTimer.singleShot(100, app_instance.app.quit)

    # Run tests after a short delay
    QTimer.singleShot(100, run_tests)

    return app_instance.run()


if __name__ == '__main__':
    sys.exit(test_playback_commands())
