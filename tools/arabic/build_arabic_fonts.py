"""
Builds the ZoneBuilder sources for the iw4x_arabic zone from an Arabic TrueType font.

Every stock font in the table below gets a replacement named "fonts/ar_<name>" containing Latin,
Arabic digits and punctuation, and the Arabic presentation forms that src/Utils/Arabic.cpp shapes
text into. Copy the output into the game folder and run "buildzone iw4x_arabic" in ZoneBuilder.

Requires fonttools (pip install fonttools).

Usage: python build_arabic_fonts.py <font.ttf> [output dir]
"""

import json
import re
import sys
from pathlib import Path

from fontTools import subset
from fontTools.ttLib import TTFont
from fontTools.varLib import instancer

ZONE_NAME = "iw4x_arabic"
ARABIC_CPP = Path(__file__).resolve().parents[2] / "src" / "Utils" / "Arabic.cpp"

# Stock font -> weight of the variable font to use. Sizes are taken from the stock font at build time.
FONTS = {
    "smallFont": 400,
    "normalFont": 400,
    "boldFont": 400,
    "bigFont": 400,
    "extraBigFont": 400,
    "objectiveFont": 400,
    "hudBigFont": 400,
    "hudSmallFont": 400,
}

# Fitting the whole font into the stock line height leaves Latin capitals at about 60% of their
# usual size because of the tall Arabic ascenders, so scale glyphs back up.
GLYPH_SCALE = 1.45

EXTRA_CODEPOINTS = [
    0x060C, 0x061B, 0x061F,  # Arabic comma, semicolon, question mark
    0x0640,  # Tatweel
    *range(0x064B, 0x0653), 0x0670,  # Harakat
    *range(0x0660, 0x066E),  # Arabic-Indic digits and number signs
    *range(0x06F0, 0x06FA),  # Persian digits
    *range(0xFEF5, 0xFEFD),  # Lam-alef ligatures
    0x00AB, 0x00BB,  # Guillemets (mirrored brackets)
]


def shaped_codepoints():
    """Presentation forms produced by the LETTERS table in Arabic.cpp."""
    source = ARABIC_CPP.read_text(encoding="utf-8")
    table = re.search(r"LETTERS\[\]\s*\{(.*?)\n\s*\};", source, re.S)
    if not table:
        sys.exit(f"Could not find the LETTERS table in {ARABIC_CPP}")

    codepoints = set()
    for forms in re.findall(r"\{0x[0-9A-Fa-f]+,\s*\{([^}]*)\}\}", table.group(1)):
        codepoints.update(int(value, 16) for value in forms.split(",") if int(value, 16))
    return codepoints


def build_charset(font):
    cmap = font.getBestCmap()
    wanted = set(range(32, 128)) | set(range(0xA0, 0x100)) | set(EXTRA_CODEPOINTS) | shaped_codepoints()

    missing = sorted(c for c in wanted if c not in cmap and c >= 0x600)
    if missing:
        print("Warning: font has no glyph for", ", ".join(f"U+{c:04X}" for c in missing))

    # ZoneBuilder requires 32-127, DEL falls back to .notdef and is never drawn
    return sorted(c for c in wanted if c in cmap or c < 128)


def make_static(source_path, weight, charset, out_path):
    font = TTFont(source_path)
    if "fvar" in font:
        font = instancer.instantiateVariableFont(font, {"wght": weight})

    options = subset.Options()
    options.layout_features = []  # Shaping is done in code, OpenType tables are not needed
    options.name_IDs = ["*"]
    options.notdef_outline = True
    subsetter = subset.Subsetter(options)
    subsetter.populate(unicodes=charset)
    subsetter.subset(font)

    font.save(out_path)


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)

    source = Path(sys.argv[1])
    out_dir = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("arabic_fonts_out")
    fonts_dir = out_dir / "userraw" / "fonts"
    zone_dir = out_dir / "zone_source"
    fonts_dir.mkdir(parents=True, exist_ok=True)
    zone_dir.mkdir(parents=True, exist_ok=True)

    charset = build_charset(TTFont(source))
    print(f"{len(charset)} glyphs per font")

    csv_lines = []
    for stock_name, weight in FONTS.items():
        name = f"ar_{stock_name}"
        make_static(source, weight, charset, fonts_dir / f"{name}.ttf")

        definition = {
            "baseFont": f"fonts/{stock_name}",
            "glyphScale": GLYPH_SCALE,
            "yOffset": 0,
            "charset": charset,
        }
        (fonts_dir / f"{name}.json").write_text(json.dumps(definition), encoding="utf-8")

        csv_lines.append(f"font,fonts/{name}")
        print(f"  fonts/{name} (weight {weight})")

    (zone_dir / f"{ZONE_NAME}.csv").write_text("\n".join(csv_lines) + "\n", encoding="utf-8")
    print(f"Wrote {out_dir}")


if __name__ == "__main__":
    main()
