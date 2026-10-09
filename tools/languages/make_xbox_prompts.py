"""
Makes the Xbox versions of the button prompts for English and Arabic.

The menus come from the PC game, whose English and Arabic text name keyboard keys ("Back - ESC").
The Xbox strings in the language pack show controller buttons instead ("\\x02 Back"). Project Xenon
maps those buttons to the keys in menus, so:
  english.json         the Xbox English strings that show buttons (loc_translation english)
  arabic.json          the Arabic translation with the same prompts turned into button prompts

Usage: python make_xbox_prompts.py <language pack folder> <arabic.json> [output dir]
"""

import json
import re
import sys
from pathlib import Path

from import_language_pack import parse_str

# Font codes of the Xbox buttons
BUTTONS = set(range(0x01, 0x07)) | set(range(0x0E, 0x18))

KEYBOARD_KEY = re.compile(r"(?<![A-Za-z])(ESCAPE|ESC|F\d{1,2}|DEL|ENTER|BACKSPACE)(?![A-Za-z0-9])")
COLOR = re.compile(r"\^\d")


def buttons_in(text):
    return [c for c in text if ord(c) in BUTTONS]


def leading_buttons(text):
    match = re.match(r"^([\x01-\x06\x0e-\x17]+)\s*", text)
    return match.group(1) if match else ""


def trailing_buttons(text):
    match = re.search(r"(\s*)([\x01-\x06\x0e-\x17]+)$", text)
    return (match.group(1), match.group(2)) if match else ("", "")


def xbox_arabic(arabic, xbox_english):
    """The Arabic text with its keyboard key replaced by the button the Xbox string shows first."""
    # Bindings (&&1, [{+activate}]) already show the button the player uses
    if "&&" in arabic or "[{" in arabic:
        return None

    leading = leading_buttons(xbox_english)
    spacing, trailing = trailing_buttons(xbox_english)
    if not leading and not trailing:
        return None

    core = COLOR.sub("", KEYBOARD_KEY.sub("", arabic))
    core = re.sub(r"^\s*-\s*|\s*-\s*$", "", core.strip()).strip()
    if not core:
        return None

    # Same order as the English text, which mirrors it in Arabic: a leading button shows on the right
    if leading:
        return f"{leading} {core}"
    return f"{core}{spacing}{trailing}"


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)

    pack = Path(sys.argv[1])
    arabic = json.loads(Path(sys.argv[2]).read_text(encoding="utf-8"))
    out = Path(sys.argv[3]) if len(sys.argv) > 3 else Path(".")
    out.mkdir(parents=True, exist_ok=True)

    xbox = parse_str(pack / "english" / "localizedstrings" / "iw4mp.str", "cp1252")
    prompts = {key: text for key, text in xbox.items() if buttons_in(text)}
    (out / "english.json").write_text(json.dumps(prompts, ensure_ascii=False, indent=1), encoding="utf-8")

    changed = 0
    for key, text in prompts.items():
        if key in arabic and (converted := xbox_arabic(arabic[key], text)):
            arabic[key] = converted
            changed += 1
    (out / "arabic.json").write_text(json.dumps(arabic, ensure_ascii=False, indent=2), encoding="utf-8")

    print(f"{len(prompts)} Xbox English prompts, {changed} Arabic prompts turned into button prompts")


if __name__ == "__main__":
    main()
