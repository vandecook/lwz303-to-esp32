"""Build-time metadata for the saved selection; never read by the ESP32."""
import json
from pathlib import Path

SELECTED_ENTITIES = json.loads(
    Path(__file__).with_name("selection.json").read_text(encoding="utf-8")
)["entities"]
