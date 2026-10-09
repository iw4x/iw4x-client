"""
Imports official Modern Warfare 2 language data (strings, font glyph tables and font textures) into
the format IW4x loads, and builds Noto backup fonts for characters the official fonts don't have.

The language pack folder has one folder per language, as dumped from the localized games:
  <language>/localizedstrings/iw4mp.str   strings in the language's legacy code page
  <language>/fonts/<font>.json            glyph tables, letters stored as legacy character codes
  <language>/images/gamefonts_pc.iwi      font texture

Output (copy into the game folder, then run "buildzone iw4x_languages" in ZoneBuilder):
  userraw/localizedstrings/<language>.json   strings as UTF-8, loaded with loc_translation <language>
  userraw/fonts/<prefix>_<font>.json         glyph tables with Unicode letters
  userraw/fonts/fb_<script>.ttf/.json        Noto backup fonts
  userraw/images/gamefonts_<prefix>.iwi      font textures
  zone_source/iw4x_languages.csv

Requires fonttools (pip install fonttools).

Usage: python import_language_pack.py <language pack folder> <Noto font folder> [output dir]
"""

import json
import re
import shutil
import sys
from pathlib import Path

from fontTools import subset
from fontTools.ttLib import TTFont
from fontTools.varLib import instancer

ZONE_NAME = "iw4x_languages"

# Folder name -> font prefix and the code page its strings and glyph codes use.
# English is the game's own text, only its console fonts are imported. Its glyph codes stay as they
# are (code page None) because the game's English text is single-byte Windows-1252. British is a copy.
LANGUAGES = {
    "english": ("en", None),
    "german": ("de", "cp1252"),
    "italian": ("it", "cp1252"),
    "spanish": ("es", "cp1252"),
    "polish": ("pl", "cp1250"),
    "russian": ("ru", "cp1251"),
    "japanese": ("ja", "cp932"),
    "korean": ("ko", "cp949"),
}

# Line height of the Noto backup fonts, the renderer scales them to the font they stand in for
BACKUP_SIZE = 28


def latin_charset():
    ranges = [(0x20, 0x7F), (0xA0, 0x250), (0x370, 0x400), (0x400, 0x530), (0x1E00, 0x1F00), (0x2000, 0x2070), (0x20A0, 0x20C0)]
    return {c for start, end in ranges for c in range(start, end)}


def legacy_charset(code_page, leads, trails):
    """Characters of a double-byte legacy code page in the given lead and trail byte ranges."""
    chars = set()
    for lead in leads:
        for trail in trails:
            try:
                text = bytes([lead, trail]).decode(code_page)
            except UnicodeDecodeError:
                continue
            if len(text) == 1:
                chars.add(ord(text))
    return chars


# Codes the console fonts use for controller button pictures rather than letters (0x01-0x1F are
# buttons too). The game sends them as raw bytes, so they keep their code in every language.
BUTTON_CODES = {0xBC, 0xBD}
BUTTON_PICTURES = set(range(0x01, 0x20)) | BUTTON_CODES

# The button backup font is made from this font: its large buttons stay sharp when scaled down
BUTTON_FONT = "bigFont"


def decode_letter(letter, code_page):
    """Glyph letters are single bytes, or lead/trail byte pairs for double-byte code pages."""
    if letter < 0x80 or letter in BUTTON_CODES or code_page is None:
        return letter
    raw = bytes([letter]) if letter <= 0xFF else bytes([letter >> 8, letter & 0xFF])
    try:
        text = raw.decode(code_page)
    except UnicodeDecodeError:
        return None
    return ord(text) if len(text) == 1 else None


# Mistakes in the official translations: language -> {key: corrected text}
FIXES = {
    # The stance hints are shifted by one in the Polish strings (jump says "lie down" and so on)
    "polish": {
        "PLATFORM_STANCEHINT_JUMP": "Naciśnij &&1, aby skoczyć",
        "PLATFORM_STANCEHINT_STAND": "Naciśnij &&1, aby wstać",
        "PLATFORM_STANCEHINT_PRONE": "Naciśnij &&1, aby się położyć",
    },
}


def parse_str(path, code_page):
    """Reads a .str file: REFERENCE <key> followed by LANG_<language> "<text>"."""
    lines = [line.rstrip(b"\r") for line in path.read_bytes().split(b"\n")]
    strings = {}
    key = None
    i = 0
    while i < len(lines):
        line = lines[i]
        i += 1
        if line.startswith(b"REFERENCE"):
            key = line.split(None, 1)[1].decode("ascii").strip()
        elif line.startswith(b"LANG_") and key and b'"' in line:
            value = line[line.index(b'"') + 1:]

            # A value continues on the next lines until its closing quote
            while not re.search(rb'(?<!\\)"\s*$', value) and i < len(lines) and not lines[i].startswith((b"REFERENCE", b"LANG_")):
                value += b"\n" + lines[i]
                i += 1

            value = re.sub(rb'"\s*$', b"", value)
            text = value.decode(code_page, errors="replace")
            strings[key] = text.replace("\\n", "\n").replace('\\"', '"')
            key = None
    return strings


def button_height(font):
    return next((g["pixelHeight"] for g in font["glyphs"] if g["letter"] == 0x01), 0)


def import_fonts(language_dir, prefix, code_page, fonts_out, english_dir):
    names = []
    for font_path in sorted((language_dir / "fonts").glob("*.json")):
        font = json.loads(font_path.read_text(encoding="utf-8-sig"))

        glyphs = {}
        for glyph in font["glyphs"]:
            letter = decode_letter(glyph["letter"], code_page)
            if letter is not None:
                glyphs[letter] = dict(glyph, letter=letter)

        stock_name = font_path.stem

        # Low resolution buttons (the Korean fonts have 20 pixel ones) come from the button backup font instead
        english_font = english_dir / "fonts" / font_path.name
        if english_font.exists():
            english_height = button_height(json.loads(english_font.read_text(encoding="utf-8-sig")))
            if 0 < button_height(font) < english_height:
                glyphs = {letter: glyph for letter, glyph in glyphs.items() if letter not in BUTTON_PICTURES}
        name = f"{prefix}_{stock_name}"
        definition = {
            "baseFont": f"fonts/{stock_name}",
            "image": f"gamefonts_{prefix}",
            "pixelHeight": font["pixelHeight"],
            "glyphs": [glyphs[letter] for letter in game_layout(glyphs)],
        }
        (fonts_out / f"{name}.json").write_text(json.dumps(definition), encoding="utf-8")
        names.append(name)
    return names


def game_layout(letters):
    """The game indexes 32-127 directly and binary searches the rest."""
    return sorted(letters, key=lambda l: (not 32 <= l <= 127, l))


def build_button_font(language_dir, fonts_out, images_out):
    """Button pictures of the English console font, for languages whose fonts don't have them (Korean)."""
    font = json.loads((language_dir / "fonts" / f"{BUTTON_FONT}.json").read_text(encoding="utf-8-sig"))
    glyphs = {g["letter"]: g for g in font["glyphs"] if 32 <= g["letter"] <= 127 or g["letter"] in BUTTON_PICTURES}
    shutil.copyfile(language_dir / "images" / "gamefonts_pc.iwi", images_out / "gamefonts_buttons.iwi")
    definition = {
        "image": "gamefonts_buttons",
        "pixelHeight": font["pixelHeight"],
        "glyphs": [glyphs[letter] for letter in game_layout(glyphs)],
    }
    (fonts_out / "fb_buttons.json").write_text(json.dumps(definition), encoding="utf-8")
    print(f"  fonts/fb_buttons: {len(glyphs)} glyphs")


def build_backup_font(source, name, charset, glyph_scale, fonts_out):
    font = TTFont(source)
    if "fvar" in font:
        # Pin every axis (Noto Sans also has width), regular weight
        axes = {axis.axisTag: axis.defaultValue for axis in font["fvar"].axes}
        axes["wght"] = 400
        font = instancer.instantiateVariableFont(font, axes)

    cmap = font.getBestCmap()
    charset = sorted(c for c in charset | set(range(32, 128)) if c in cmap or c < 128)

    options = subset.Options()
    options.layout_features = []
    options.name_IDs = ["*"]
    options.notdef_outline = True
    subsetter = subset.Subsetter(options)
    subsetter.populate(unicodes=charset)
    subsetter.subset(font)
    font.save(fonts_out / f"{name}.ttf")

    definition = {"size": BACKUP_SIZE, "glyphScale": glyph_scale, "yOffset": 0, "charset": charset}
    (fonts_out / f"{name}.json").write_text(json.dumps(definition), encoding="utf-8")
    print(f"  fonts/{name}: {len(charset)} glyphs")


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)

    pack = Path(sys.argv[1])
    noto = Path(sys.argv[2])
    out = Path(sys.argv[3]) if len(sys.argv) > 3 else Path("language_pack_out")
    fonts_out = out / "userraw" / "fonts"
    images_out = out / "userraw" / "images"
    strings_out = out / "userraw" / "localizedstrings"
    for folder in (fonts_out, images_out, strings_out, out / "zone_source"):
        folder.mkdir(parents=True, exist_ok=True)

    zone_fonts = []
    used_chars = {"ja": set(), "ko": set()}
    for language, (prefix, code_page) in LANGUAGES.items():
        language_dir = pack / language
        if not language_dir.exists():
            print(f"Skipping {language}, not found")
            continue

        strings = {}
        if code_page is not None:
            strings = parse_str(language_dir / "localizedstrings" / "iw4mp.str", code_page)
            strings.update(FIXES.get(language, {}))
            (strings_out / f"{language}.json").write_text(json.dumps(strings, ensure_ascii=False, indent=1), encoding="utf-8")
        if prefix in used_chars:
            used_chars[prefix].update(ord(c) for text in strings.values() for c in text)

        shutil.copyfile(language_dir / "images" / "gamefonts_pc.iwi", images_out / f"gamefonts_{prefix}.iwi")
        names = import_fonts(language_dir, prefix, code_page, fonts_out, pack / "english")
        zone_fonts += names
        print(f"{language}: {len(strings)} strings, {len(names)} fonts")

    # Backup fonts cover what players type in chat as well as the game text
    print("Backup fonts:")
    build_button_font(pack / "english", fonts_out, images_out)
    build_backup_font(noto / "NotoSans.ttf", "fb_latin", latin_charset(), 1.0, fonts_out)
    # Shift-JIS symbols, kana and level 1 kanji (lead bytes up to 0x98); KS X 1001 symbols and the 2350 common syllables
    build_backup_font(noto / "NotoSansJP.ttf", "fb_ja", legacy_charset("cp932", range(0x81, 0x99), range(0x40, 0xFD)) | used_chars["ja"], 1.0, fonts_out)
    build_backup_font(noto / "NotoSansKR.ttf", "fb_ko", legacy_charset("cp949", range(0xA1, 0xC9), range(0xA1, 0xFF)) | used_chars["ko"], 1.0, fonts_out)
    zone_fonts += ["fb_buttons", "fb_latin", "fb_ja", "fb_ko"]

    csv = "".join(f"font,fonts/{name}\n" for name in zone_fonts)
    (out / "zone_source" / f"{ZONE_NAME}.csv").write_text(csv, encoding="utf-8")
    print(f"Wrote {out} ({len(zone_fonts)} fonts)")


if __name__ == "__main__":
    main()
