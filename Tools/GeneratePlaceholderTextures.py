#!/usr/bin/env python3
"""
GeneratePlaceholderTextures.py
Naruto: Ultimate Shinobi Legacy — Placeholder Texture Generator

Generates 512x512 placeholder portrait and full-body PNG textures for every
character in the registry. Each character gets a unique solid color derived
from their village affiliation, with their name drawn in white text.

These images are PLACEHOLDERS only. Replace them by importing real
artwork into UE5 at the same Content Browser path.

Usage:
    python3 Tools/GeneratePlaceholderTextures.py

Requirements:
    pip install pillow

Output:
    Content/Characters/<Name>/Textures/T_<Name>_Portrait.png
    Content/Characters/<Name>/Textures/T_<Name>_FullBody.png

After running this script:
    1. Open UE5 editor
    2. Right-click Content/Characters in the Content Browser
    3. Select "Import" and batch-import all generated PNGs
    4. The data assets will automatically resolve the texture references.
"""

import os
import sys

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    print("Pillow not installed. Run: pip install pillow")
    sys.exit(1)

# ============================================================
#  Village colors (background tint for each character's card)
# ============================================================
VILLAGE_COLORS = {
    "Konohagakure": (34, 85, 34),       # Forest green
    "Sunagakure":   (194, 156, 86),     # Sand gold
    "Kirigakure":   (70, 110, 130),     # Mist blue-grey
    "Kumogakure":   (80, 80, 160),      # Cloud purple-blue
    "Iwagakure":    (100, 80, 60),      # Stone brown
    "Akatsuki":     (60, 0, 0),         # Dark crimson
    "Otogakure":    (80, 40, 100),      # Sound purple
    "Amegakure":    (50, 50, 80),       # Rain dark blue
    "Otsutsuki":    (150, 100, 200),    # Celestial purple
    "None":         (40, 40, 40),       # Neutral dark
}

# ============================================================
#  All 150 characters: (tag_name, display_name, village)
# ============================================================
CHARACTERS = [
    # Leaf
    ("Naruto",          "Naruto Uzumaki",       "Konohagakure"),
    ("Sasuke",          "Sasuke Uchiha",        "Konohagakure"),
    ("Sakura",          "Sakura Haruno",        "Konohagakure"),
    ("Kakashi",         "Kakashi Hatake",       "Konohagakure"),
    ("RockLee",         "Rock Lee",             "Konohagakure"),
    ("Neji",            "Neji Hyuga",           "Konohagakure"),
    ("Tenten",          "Tenten",               "Konohagakure"),
    ("Gai",             "Might Guy",            "Konohagakure"),
    ("Shikamaru",       "Shikamaru Nara",       "Konohagakure"),
    ("Ino",             "Ino Yamanaka",         "Konohagakure"),
    ("Choji",           "Choji Akimichi",       "Konohagakure"),
    ("Hinata",          "Hinata Hyuga",         "Konohagakure"),
    ("Kiba",            "Kiba Inuzuka",         "Konohagakure"),
    ("Shino",           "Shino Aburame",        "Konohagakure"),
    ("Asuma",           "Asuma Sarutobi",       "Konohagakure"),
    ("Kurenai",         "Kurenai Yuhi",         "Konohagakure"),
    ("Tsunade",         "Tsunade Senju",        "Konohagakure"),
    ("Jiraiya",         "Jiraiya",              "Konohagakure"),
    ("Minato",          "Minato Namikaze",      "Konohagakure"),
    ("Kushina",         "Kushina Uzumaki",      "Konohagakure"),
    ("Hiruzen",         "Hiruzen Sarutobi",     "Konohagakure"),
    ("Itachi",          "Itachi Uchiha",        "Akatsuki"),
    ("Shisui",          "Shisui Uchiha",        "Konohagakure"),
    ("Yamato",          "Yamato",               "Konohagakure"),
    ("Sai",             "Sai",                  "Konohagakure"),
    ("Anko",            "Anko Mitarashi",       "Konohagakure"),
    ("Ibiki",           "Ibiki Morino",         "Konohagakure"),
    ("Konohamaru",      "Konohamaru Sarutobi",  "Konohagakure"),
    ("Iruka",           "Iruka Umino",          "Konohagakure"),
    ("HashiramaFirst",  "Hashirama Senju",      "Konohagakure"),
    ("TobiramaSecond",  "Tobirama Senju",       "Konohagakure"),
    ("HiruzenPrime",    "Hiruzen (Prime)",      "Konohagakure"),
    ("MinatoKCM",       "Minato (KCM)",         "Konohagakure"),
    ("MightGai8G",      "Guy (8th Gate)",       "Konohagakure"),
    ("NarutoSage",      "Naruto (Sage Mode)",   "Konohagakure"),
    ("NarutoKCM",       "Naruto (KCM)",         "Konohagakure"),
    ("NarutoSPSM",      "Naruto (SPSM)",        "Konohagakure"),
    ("NarutoBaryon",    "Naruto (Baryon)",      "Konohagakure"),
    ("SasukeCS2",       "Sasuke (CS2)",         "Akatsuki"),
    ("SasukeRinnegan",  "Sasuke (Rinnegan)",    "Konohagakure"),
    ("SasukeAdult",     "Sasuke (Adult)",       "Konohagakure"),
    ("KakashiDMS",      "Kakashi (DMS)",        "Konohagakure"),
    ("SakuraAdult",     "Sakura (Adult)",       "Konohagakure"),
    ("RockLeeGates",    "Lee (Gates)",          "Konohagakure"),
    ("NejiByakugan",    "Neji (Byakugan)",      "Konohagakure"),
    ("ShikamaruAdult",  "Shikamaru (Adult)",    "Konohagakure"),
    ("HinataAdult",     "Hinata (Adult)",       "Konohagakure"),
    ("ChojiButterfly",  "Choji (Butterfly)",    "Konohagakure"),
    ("Danzo",           "Danzo Shimura",        "Konohagakure"),
    ("KabutoSage",      "Kabuto (Sage)",        "Otogakure"),
    ("ObiteYoung",      "Obito (Young)",        "Konohagakure"),
    ("KakashiYoung",    "Kakashi (Young)",      "Konohagakure"),
    ("Rin",             "Rin Nohara",           "Konohagakure"),
    ("Boruto",          "Boruto Uzumaki",       "Konohagakure"),
    ("Sarada",          "Sarada Uchiha",        "Konohagakure"),
    # Sand
    ("Gaara",           "Gaara of the Sand",    "Sunagakure"),
    ("Temari",          "Temari",               "Sunagakure"),
    ("Kankuro",         "Kankuro",              "Sunagakure"),
    ("Chiyo",           "Chiyo-baa",            "Sunagakure"),
    ("Rasa",            "Rasa (4th Kazekage)",  "Sunagakure"),
    ("GaaraKage",       "Gaara (Kazekage)",     "Sunagakure"),
    ("Pakura",          "Pakura",               "Sunagakure"),
    # Mist
    ("Zabuza",          "Zabuza Momochi",       "Kirigakure"),
    ("Haku",            "Haku",                 "Kirigakure"),
    ("Kisame",          "Kisame Hoshigaki",     "Akatsuki"),
    ("Mei",             "Mei Terumi",           "Kirigakure"),
    ("Suigetsu",        "Suigetsu Hozuki",      "Kirigakure"),
    ("Jugo",            "Jugo",                 "Kirigakure"),
    ("Karin",           "Karin",                "Otogakure"),
    ("Chojuro",         "Chojuro",              "Kirigakure"),
    ("Ao",              "Ao",                   "Kirigakure"),
    # Cloud
    ("KillerBee",       "Killer Bee",           "Kumogakure"),
    ("Raikage",         "A (4th Raikage)",      "Kumogakure"),
    ("Darui",           "Darui",                "Kumogakure"),
    ("Samui",           "Samui",                "Kumogakure"),
    ("Omoi",            "Omoi",                 "Kumogakure"),
    ("Karui",           "Karui",                "Kumogakure"),
    ("KillerBeeV2",     "Killer Bee (V2)",      "Kumogakure"),
    # Stone
    ("Deidara",         "Deidara",              "Akatsuki"),
    ("Onoki",           "Onoki (3rd Tsuchikage)","Iwagakure"),
    ("Kurotsuchi",      "Kurotsuchi",           "Iwagakure"),
    # Akatsuki
    ("PainNagato",      "Pain / Nagato",        "Akatsuki"),
    ("Konan",           "Konan",                "Akatsuki"),
    ("TobiObito",       "Tobi / Obito",         "Akatsuki"),
    ("Madara",          "Madara Uchiha",        "Akatsuki"),
    ("Sasori",          "Sasori",               "Akatsuki"),
    ("Hidan",           "Hidan",                "Akatsuki"),
    ("Kakuzu",          "Kakuzu",               "Akatsuki"),
    ("ZetsuWhite",      "White Zetsu",          "Akatsuki"),
    ("ZetsuBlack",      "Black Zetsu",          "Akatsuki"),
    ("DevaPath",        "Deva Path",            "Akatsuki"),
    ("AsuraPath",       "Asura Path",           "Akatsuki"),
    ("HumanPath",       "Human Path",           "Akatsuki"),
    ("AnimalPath",      "Animal Path",          "Akatsuki"),
    ("PretaPath",       "Preta Path",           "Akatsuki"),
    ("NarakaPath",      "Naraka Path",          "Akatsuki"),
    # Otsutsuki
    ("Kaguya",          "Kaguya Otsutsuki",     "Otsutsuki"),
    ("Momoshiki",       "Momoshiki Otsutsuki",  "Otsutsuki"),
    ("Kinshiki",        "Kinshiki Otsutsuki",   "Otsutsuki"),
    ("Hamura",          "Hamura Otsutsuki",     "Otsutsuki"),
    ("Hagoromo",        "Hagoromo Otsutsuki",   "Otsutsuki"),
    # Other
    ("Orochimaru",      "Orochimaru",           "Otogakure"),
    ("Kabuto",          "Kabuto Yakushi",       "Otogakure"),
    ("Yahiko",          "Yahiko",               "Amegakure"),
    ("NagatoYoung",     "Nagato (Young)",       "Amegakure"),
    ("KonanYoung",      "Konan (Young)",        "Amegakure"),
    ("Mifune",          "Mifune",               "None"),
    ("KuramaFull",      "Kurama (Full)",        "Konohagakure"),
    ("JuubiJin",        "Juubi Jinchuuriki",    "Otsutsuki"),
]

# ============================================================
#  Generator
# ============================================================

def hex_to_rgb(h):
    h = h.lstrip('#')
    return tuple(int(h[i:i+2], 16) for i in (0, 2, 4))

def lighten(color, amount=60):
    return tuple(min(255, c + amount) for c in color)

def draw_placeholder(name, display_name, village, size, label):
    """
    Creates a placeholder image:
      - Solid village-colored background
      - Diagonal stripe pattern (darker shade)
      - Character name centered in white
      - 'PLACEHOLDER' watermark at the bottom
      - Village name at the top
    """
    bg_color  = VILLAGE_COLORS.get(village, VILLAGE_COLORS["None"])
    stripe_color = tuple(max(0, c - 30) for c in bg_color)

    img  = Image.new("RGB", (size, size), bg_color)
    draw = ImageDraw.Draw(img)

    # Diagonal stripes
    step = 40
    for i in range(-size, size * 2, step):
        draw.line([(i, 0), (i + size, size)], fill=stripe_color, width=2)

    # Top bar
    draw.rectangle([(0, 0), (size, 50)], fill=tuple(max(0, c - 50) for c in bg_color))

    # Bottom bar
    draw.rectangle([(0, size - 40), (size, size)],
                   fill=tuple(max(0, c - 50) for c in bg_color))

    # Try to load a font — fall back to default if not available
    try:
        title_font = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 28)
        sub_font   = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 16)
        small_font = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 13)
    except Exception:
        title_font = ImageFont.load_default()
        sub_font   = title_font
        small_font = title_font

    # Village name top bar
    draw.text((size // 2, 25), village, fill=(255, 255, 220),
              font=sub_font, anchor="mm")

    # Character display name centered
    draw.text((size // 2, size // 2 - 20), display_name,
              fill=(255, 255, 255), font=title_font, anchor="mm")

    # Label (Portrait / Full Body)
    draw.text((size // 2, size // 2 + 20), f"[{label}]",
              fill=(200, 200, 200), font=sub_font, anchor="mm")

    # Placeholder watermark bottom bar
    draw.text((size // 2, size - 20), "PLACEHOLDER — REPLACE WITH REAL ART",
              fill=(180, 180, 180), font=small_font, anchor="mm")

    return img


def generate_all():
    script_dir   = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    content_root = os.path.join(project_root, "Content", "Characters")

    generated = 0
    skipped   = 0

    for name, display_name, village in CHARACTERS:
        tex_dir = os.path.join(content_root, name, "Textures")
        os.makedirs(tex_dir, exist_ok=True)

        portrait_path   = os.path.join(tex_dir, f"T_{name}_Portrait.png")
        fullbody_path   = os.path.join(tex_dir, f"T_{name}_FullBody.png")

        if not os.path.exists(portrait_path):
            img = draw_placeholder(name, display_name, village, 512, "Portrait")
            img.save(portrait_path, "PNG")
            generated += 1
        else:
            skipped += 1

        if not os.path.exists(fullbody_path):
            img = draw_placeholder(name, display_name, village, 512, "Full Body")
            img.save(fullbody_path, "PNG")
            generated += 1
        else:
            skipped += 1

    print(f"\n✓ Done. Generated {generated} placeholder textures, "
          f"skipped {skipped} existing files.")
    print(f"  Output: {content_root}")
    print(f"\n  Next step: In UE5, right-click Content/Characters → Import to bulk-import all PNGs.")


if __name__ == "__main__":
    generate_all()
