#!/usr/bin/env python3
"""
Test script to verify the three code review fixes:
1. Error handling for missing agent fields
2. Configurable world size
3. Blue→yellow colormap
"""

import sys
import numpy as np
from PyQt6.QtWidgets import QApplication
from widgets.global_view import GlobalViewWidget

def test_error_handling(app):
    """Test 1: Error handling for missing fields"""
    print("Test 1: Error handling for missing agent fields...")

    widget = GlobalViewWidget()

    # Test with incomplete data (missing 'haze_mean')
    incomplete_agents = [
        {'agent_id': 1, 'x': 1.0, 'y': 1.0, 'vx': 0.1, 'vy': 0.1, 'fatigue': 0.3},
        {'agent_id': 2, 'x': 2.0, 'y': 2.0, 'vx': -0.1, 'vy': 0.1, 'fatigue': 0.5},
    ]

    # This should not crash, should print warning instead
    try:
        widget.update_agents(incomplete_agents)
        print("  ✓ No crash with incomplete data (warning should be printed above)")
    except Exception as e:
        print(f"  ✗ FAILED: Crashed with {e}")
        return False

    return True

def test_configurable_world_size(app):
    """Test 2: Configurable world size"""
    print("\nTest 2: Configurable world size...")

    # Test initialization with custom world size
    widget = GlobalViewWidget(world_size=(20.0, 15.0))

    if widget.world_size == (20.0, 15.0):
        print(f"  ✓ Constructor accepts world_size: {widget.world_size}")
    else:
        print(f"  ✗ FAILED: Expected (20.0, 15.0), got {widget.world_size}")
        return False

    # Test set_world_size method
    widget.set_world_size(30.0, 25.0)

    if widget.world_size == (30.0, 25.0):
        print(f"  ✓ set_world_size() works: {widget.world_size}")
    else:
        print(f"  ✗ FAILED: Expected (30.0, 25.0), got {widget.world_size}")
        return False

    # Verify plot limits updated
    xlim = widget.ax.get_xlim()
    ylim = widget.ax.get_ylim()

    if xlim == (0, 30.0) and ylim == (0, 25.0):
        print(f"  ✓ Plot limits updated: xlim={xlim}, ylim={ylim}")
    else:
        print(f"  ✗ FAILED: Expected xlim=(0, 30.0), ylim=(0, 25.0)")
        print(f"            Got xlim={xlim}, ylim={ylim}")
        return False

    return True

def test_colormap(app):
    """Test 3: Blue→yellow colormap"""
    print("\nTest 3: Blue→yellow colormap...")

    widget = GlobalViewWidget()

    # Check that custom colormap exists
    if not hasattr(widget, 'haze_cmap'):
        print("  ✗ FAILED: haze_cmap not found")
        return False

    print(f"  ✓ Custom colormap created: {widget.haze_cmap.name}")

    # Test colormap colors
    # At value 0.0 (low haze) should be blue
    # At value 1.0 (high haze) should be yellow

    color_at_0 = widget.haze_cmap(0.0)  # RGBA tuple
    color_at_1 = widget.haze_cmap(1.0)  # RGBA tuple

    # Blue is (0, 0, 1) in RGB
    # Yellow is (1, 1, 0) in RGB

    print(f"  Color at 0.0 (low haze): RGB = ({color_at_0[0]:.2f}, {color_at_0[1]:.2f}, {color_at_0[2]:.2f})")
    print(f"  Color at 1.0 (high haze): RGB = ({color_at_1[0]:.2f}, {color_at_1[1]:.2f}, {color_at_1[2]:.2f})")

    # Check if blue-ish at 0 (low R, low G, high B)
    if color_at_0[0] < 0.3 and color_at_0[1] < 0.3 and color_at_0[2] > 0.7:
        print("  ✓ Low haze (0.0) is blue")
    else:
        print("  ✗ FAILED: Low haze should be blue")
        return False

    # Check if yellow-ish at 1 (high R, high G, low B)
    if color_at_1[0] > 0.7 and color_at_1[1] > 0.7 and color_at_1[2] < 0.3:
        print("  ✓ High haze (1.0) is yellow")
    else:
        print("  ✗ FAILED: High haze should be yellow")
        return False

    return True

def test_rendering_with_valid_data(app):
    """Bonus test: Verify rendering works with valid data"""
    print("\nBonus Test: Rendering with valid data...")

    widget = GlobalViewWidget()

    # Create valid test data
    valid_agents = [
        {'agent_id': i,
         'x': np.random.uniform(0, 10),
         'y': np.random.uniform(0, 10),
         'vx': np.random.uniform(-0.5, 0.5),
         'vy': np.random.uniform(-0.5, 0.5),
         'haze_mean': np.random.uniform(0, 1),
         'fatigue': np.random.uniform(0, 1)}
        for i in range(10)
    ]

    try:
        widget.update_agents(valid_agents)
        print("  ✓ Rendering with valid data succeeded")
        return True
    except Exception as e:
        print(f"  ✗ FAILED: {e}")
        return False

if __name__ == '__main__':
    print("="*60)
    print("GlobalViewWidget Code Review Fixes - Verification Tests")
    print("="*60)

    # Create QApplication once
    app = QApplication(sys.argv)

    results = []

    results.append(test_error_handling(app))
    results.append(test_configurable_world_size(app))
    results.append(test_colormap(app))
    results.append(test_rendering_with_valid_data(app))

    print("\n" + "="*60)
    if all(results):
        print("✓ ALL TESTS PASSED")
        print("="*60)
        sys.exit(0)
    else:
        print("✗ SOME TESTS FAILED")
        print("="*60)
        sys.exit(1)
