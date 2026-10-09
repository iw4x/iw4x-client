"""
Extracts the English localized strings from a Modern Warfare 2 installation.

Writes english.json (reference text) and creates or updates arabic.json, the file translators fill in.
Existing translations are kept, new keys are added with an empty value and keys that no longer
exist are reported.

Strings whose text is shared with another key are stored as a pointer and are not found here, and
neither are the IW4x strings. For the complete list run "iw4x.exe -zonebuilder -stdout +loc_dumpStrings",
which writes every loaded string to userraw/localizedstrings/dump.json.

Usage: python extract_strings.py <MW2 folder> [output dir]
"""

import json
import re
import sys
import zlib
from pathlib import Path

# Zones that hold localize entries, in load order so later zones override earlier ones
ZONES = [
    "localized_code_pre_gfx_mp",
    "localized_code_post_gfx_mp",
    "localized_common_mp",
    "localized_ui_mp",
    "patch_mp",
]

# Signed fastfiles: 0x15 byte header, 0x2000 byte signature header, then 0x2000 byte hash blocks
# in front of every 2 MB of zlib data
DATA_START = 0x4015
BLOCK_SIZE = 0x200000
HASH_BLOCK_SIZE = 0x2000

# Inline LocalizeEntry: value and name pointers set to -1, followed by both strings
ENTRY_PATTERN = re.compile(rb"\xff{8}([^\x00\xff]*)\x00([A-Z0-9_]{2,})\x00")


def decompress_zone(path):
    data = path.read_bytes()
    if not data.startswith(b"IWff0100"):
        raise ValueError(f"{path.name} is not a signed IW4 fastfile")

    decompressor = zlib.decompressobj()
    out = []
    pos = DATA_START
    while pos < len(data) and not decompressor.eof:
        out.append(decompressor.decompress(data[pos:pos + BLOCK_SIZE]))
        pos += BLOCK_SIZE + HASH_BLOCK_SIZE
    return b"".join(out)


def extract(game_dir, language="english"):
    strings = {}
    for zone in ZONES:
        path = game_dir / "zone" / language / f"{zone}.ff"
        if not path.exists():
            print(f"Skipping missing zone {path}")
            continue

        entries = ENTRY_PATTERN.findall(decompress_zone(path))
        for value, key in entries:
            strings[key.decode()] = value.decode("cp1252", errors="replace")
        print(f"{zone}: {len(entries)} strings")
    return strings


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)

    game_dir = Path(sys.argv[1])
    out_dir = Path(sys.argv[2]) if len(sys.argv) > 2 else Path(".")
    out_dir.mkdir(parents=True, exist_ok=True)

    english = dict(sorted(extract(game_dir).items()))
    (out_dir / "english.json").write_text(json.dumps(english, ensure_ascii=False, indent=2), encoding="utf-8")

    arabic_path = out_dir / "arabic.json"
    arabic = json.loads(arabic_path.read_text(encoding="utf-8")) if arabic_path.exists() else {}

    stale = sorted(set(arabic) - set(english))
    if stale:
        print(f"{len(stale)} translated keys are not in the game: {', '.join(stale[:10])}{' ...' if len(stale) > 10 else ''}")

    merged = {key: arabic.get(key, "") for key in english}
    merged.update({key: arabic[key] for key in stale})
    arabic_path.write_text(json.dumps(merged, ensure_ascii=False, indent=2), encoding="utf-8")

    done = sum(1 for value in merged.values() if value)
    print(f"{len(english)} strings, {done} translated ({done * 100 // max(len(english), 1)}%)")


if __name__ == "__main__":
    main()
