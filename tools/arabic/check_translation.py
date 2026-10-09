"""
Checks arabic.json against english.json for mistakes that break strings in game.

Errors: placeholders the game fills in (&&1, %s, key bindings like [{+attack}]) that are missing or
added. Warnings: different color codes or line breaks, and values still in English.

Usage: python check_translation.py [folder with english.json and arabic.json]
"""

import json
import re
import sys
from collections import Counter
from pathlib import Path

PLACEHOLDERS = re.compile(r"&&\d|%[sdif]|\[\{[^}]*\}\]")
COLORS = re.compile(r"\^\d")
ARABIC = re.compile(r"[؀-ۿ]")
LATIN_WORD = re.compile(r"[A-Za-z]{4,}")


def main():
    folder = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(".")
    english = json.loads((folder / "english.json").read_text(encoding="utf-8"))
    arabic = json.loads((folder / "arabic.json").read_text(encoding="utf-8"))

    errors = warnings = translated = 0
    for key, value in arabic.items():
        if not value:
            continue
        translated += 1

        if key not in english:
            print(f"WARN  {key}: not a game string")
            warnings += 1
            continue

        source = english[key]
        if Counter(PLACEHOLDERS.findall(source)) != Counter(PLACEHOLDERS.findall(value)):
            print(f"ERROR {key}: placeholders {PLACEHOLDERS.findall(source)} became {PLACEHOLDERS.findall(value)}")
            errors += 1

        if Counter(COLORS.findall(source)) != Counter(COLORS.findall(value)):
            print(f"WARN  {key}: color codes {COLORS.findall(source)} became {COLORS.findall(value)}")
            warnings += 1

        if source.count("\n") != value.count("\n"):
            print(f"WARN  {key}: {source.count(chr(10))} line breaks became {value.count(chr(10))}")
            warnings += 1

        if LATIN_WORD.search(source) and not ARABIC.search(value):
            print(f"WARN  {key}: looks untranslated: {value!r}")
            warnings += 1

    print(f"{translated}/{len(english)} translated, {errors} errors, {warnings} warnings")
    sys.exit(1 if errors else 0)


if __name__ == "__main__":
    main()
