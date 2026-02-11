# pytest configuration for EPH v2.1 tests

import sys
from pathlib import Path

# Add src/ directory to Python path so tests can import from gui, etc.
src_path = Path(__file__).parent.parent / "src"
sys.path.insert(0, str(src_path))
