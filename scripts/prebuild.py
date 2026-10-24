"""PlatformIO PRE script: generate ignored firmware assets and secrets headers.

This uses the Python standard library only. It never prints secret values.
The ESP32 does not read a .env at runtime: the build embeds the values into
flash memory, so protect the device/compiled firmware as well.
"""

from pathlib import Path
import json

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "include"
WEB = ROOT / "web"

REQUIRED = (
    "WIFI_SSID",
    "WIFI_PASS",
    "GREEN_API_URL",
    "GREEN_ID_INSTANCE",
    "GREEN_API_TOKEN",
    "DOCTOR_CHAT_ID",
    "FAMILY_CHAT_ID",
)


def parse_env(path):
    if not path.is_file():
        raise RuntimeError("Missing .env. Copy .env.example to .env and fill in your credentials.")
    values = {}
    for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if line.startswith("export "):
            line = line[7:]
        if "=" not in line:
            raise RuntimeError(f"Invalid .env assignment at line {line_number}")
        key, value = (part.strip() for part in line.split("=", 1))
        if value.startswith(('"', "'")) and value.endswith(value[0]):
            value = value[1:-1]
        values[key] = value
    missing = [key for key in REQUIRED if not values.get(key)]
    if missing:
        raise RuntimeError("Missing/empty .env keys: " + ", ".join(missing))
    return values


def write_if_changed(path, text):
    if not path.exists() or path.read_text(encoding="utf-8") != text:
        path.write_text(text, encoding="utf-8")


def embed_cpp_string(value):
    # JSON quoting produces C++-compatible escaped string literals for typical ASCII credentials.
    return json.dumps(value, ensure_ascii=True)


def generate():
    OUT.mkdir(parents=True, exist_ok=True)
    values = parse_env(ROOT / ".env")
    secret_lines = ["// AUTO-GENERATED — DO NOT COMMIT OR EDIT.", "#pragma once", "namespace Secrets {"]
    secret_lines += [f"static const char {key}[] = {embed_cpp_string(values[key])};" for key in REQUIRED]
    secret_lines += ["} // namespace Secrets", ""]
    write_if_changed(OUT / "GeneratedSecrets.h", "\n".join(secret_lines))

    assets = [
        ("INDEX_HTML", WEB / "index.html"),
        ("STYLES_CSS", WEB / "styles.css"),
        ("APP_JS", WEB / "app.js"),
    ]
    asset_lines = ["// AUTO-GENERATED — DO NOT EDIT.", "#pragma once", "#include <Arduino.h>", "namespace Assets {"]
    for name, path in assets:
        content = path.read_text(encoding="utf-8")
        delimiter = "ESP32ASSET"
        if f"){delimiter}\"" in content:
            raise RuntimeError(f"Raw-string delimiter collision in {path.name}")
        asset_lines.append(f'static const char {name}[] PROGMEM = R"{delimiter}({content}){delimiter}";')
    asset_lines += ["} // namespace Assets", ""]
    write_if_changed(OUT / "GeneratedAssets.h", "\n".join(asset_lines))
    print("Generated local credential header and embedded dashboard assets.")


generate()
