"""Download one light-data chunk and save raw and decoded JSON files."""

import json
import re
from pathlib import Path
from urllib.request import urlopen


PLAYER_NUM = 0
CHUNK_NUM = 0

API_URL = (
    "https://eesa.dece.nycu.edu.tw/lightdance/api/items/eesa3/"
    f"LATEST/player={PLAYER_NUM}/chunk={CHUNK_NUM}"
)

# Both files are created beside this Python script.
RAW_OUTPUT_FILE = Path(__file__).with_name("lightdata_raw.json")
DECODED_OUTPUT_FILE = Path(__file__).with_name("lightdata_decoded.json")

def decode_light_data(value):
    value = int(value)

    color = (value >> 8) & 0xFFFFFF
    brightness_level = (value >> 4) & 0x0F

    red = (color >> 16) & 0xFF
    green = (color >> 8) & 0xFF
    blue = color & 0xFF

    brightness_255 = round((brightness_level / 15) ** 2.2 * 255)

    return {
        "rgb": [red, green, blue],
        "brightness_level": brightness_level,
        "brightness_255": brightness_255,
        "transition": bool(value & 1),
    }


with urlopen(API_URL, timeout=15) as response:
    data = json.load(response)

# Save the unchanged response from the API.
with RAW_OUTPUT_FILE.open("w", encoding="utf-8") as file:
    json.dump(data, file, indent=2, ensure_ascii=False)
    file.write("\n")

# Convert packed body-part values into readable fields.
decoded_frames = []

for frame in data.get("player_data", []):
    time_ticks = int(frame.get("time", 0))
    time_label = f"time: {time_ticks * 50} ms"
    decoded_frame = {time_label: {}}

    # Every field except "time" is a packed light value. This automatically
    # supports new or renamed body parts without changing this script.
    for name, value in frame.items():
        if name != "time":
            decoded_frame[time_label][name] = decode_light_data(value)

    decoded_frames.append(decoded_frame)

decoded_data = data.copy()
decoded_data["player_data"] = decoded_frames

with DECODED_OUTPUT_FILE.open("w", encoding="utf-8") as file:
    decoded_json = json.dumps(decoded_data, indent=2, ensure_ascii=False)

    # Keep each RGB array on one line: "rgb": [255, 0, 0]
    decoded_json = re.sub(
        r'("rgb": )\[\s*(\d+),\s*(\d+),\s*(\d+)\s*\]',
        r'\1[\2, \3, \4]',
        decoded_json,
    )

    file.write(decoded_json + "\n")
