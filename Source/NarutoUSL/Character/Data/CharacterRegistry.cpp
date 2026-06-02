// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Character/Data/CharacterRegistry.h"

// ============================================================
//  Queries
// ============================================================

const FCharacterRegistryEntry* UCharacterRegistry::FindByTag(FGameplayTag Tag) const
{
    return Characters.FindByPredicate([&Tag](const FCharacterRegistryEntry& E)
    {
        return E.CharacterTag == Tag;
    });
}

TArray<FCharacterRegistryEntry>
UCharacterRegistry::GetCharactersByVillage(EVillage Village) const
{
    return Characters.FilterByPredicate([Village](const FCharacterRegistryEntry& E)
    {
        return E.Village == Village;
    });
}

TArray<FCharacterRegistryEntry> UCharacterRegistry::GetUnlockedCharacters() const
{
    return Characters.FilterByPredicate([](const FCharacterRegistryEntry& E)
    {
        return E.bUnlocked;
    });
}

// ============================================================
//  Macro helpers for building entries cleanly
// ============================================================

// CHAR(TagStr, Name, Village, Rank, bUnlocked)
// All asset paths follow the convention:
//   /Game/Characters/<Name>/DA_Character_<Name>
//   /Game/Characters/<Name>/Textures/T_<Name>_Portrait
//   /Game/Characters/<Name>/Textures/T_<Name>_FullBody
//   /Game/Characters/<Name>/Meshes/SK_<Name>
//   /Game/Characters/<Name>/Meshes/SK_<Name>_FPArms
//   /Game/Characters/<Name>/ABP_<Name>
//
// IMPORTANT: These paths are PLACEHOLDERS.
// Import real assets at these content paths in the UE5 editor.
// The engine renders a checkerboard until the real asset exists.

#define CHAR_ENTRY(TagStr, NameStr, VillageVal, RankVal, UnlockedVal)          \
    {                                                                           \
        FGameplayTag::RequestGameplayTag(TEXT(TagStr)),                        \
        FText::FromString(TEXT(NameStr)),                                      \
        FSoftObjectPath(TEXT("/Game/Characters/" NameStr                       \
            "/DA_Character_" NameStr "." "DA_Character_" NameStr)),            \
        FSoftObjectPath(TEXT("/Game/Characters/" NameStr                       \
            "/Textures/T_" NameStr "_Portrait.T_" NameStr "_Portrait")),       \
        FSoftObjectPath(TEXT("/Game/Characters/" NameStr                       \
            "/Textures/T_" NameStr "_FullBody.T_" NameStr "_FullBody")),       \
        FSoftObjectPath(TEXT("/Game/Characters/" NameStr                       \
            "/Meshes/SK_" NameStr "_FPArms.SK_" NameStr "_FPArms")),          \
        FSoftObjectPath(TEXT("/Game/Characters/" NameStr                       \
            "/Meshes/SK_" NameStr ".SK_" NameStr)),                            \
        FSoftObjectPath(TEXT("/Game/Characters/" NameStr                       \
            "/ABP_" NameStr ".ABP_" NameStr)),                                 \
        EVillage::VillageVal,                                                  \
        EShinobi_Rank::RankVal,                                                \
        UnlockedVal,                                                           \
        true                                                                   \
    }

// ============================================================
//  Default Registry — all 150 characters
// ============================================================

TArray<FCharacterRegistryEntry> UCharacterRegistry::BuildDefaultRegistry()
{
    TArray<FCharacterRegistryEntry> Registry;
    Registry.Reserve(150);

    // ----------------------------------------------------------------
    //  HIDDEN LEAF VILLAGE — Konohagakure
    // ----------------------------------------------------------------
    Registry.Add(CHAR_ENTRY("Character.Naruto",         "Naruto",         Konohagakure, Genin,        true));
    Registry.Add(CHAR_ENTRY("Character.Sasuke",         "Sasuke",         Konohagakure, Genin,        true));
    Registry.Add(CHAR_ENTRY("Character.Sakura",         "Sakura",         Konohagakure, Genin,        false));
    Registry.Add(CHAR_ENTRY("Character.Kakashi",        "Kakashi",        Konohagakure, Jonin,        false));
    Registry.Add(CHAR_ENTRY("Character.Rock_Lee",       "RockLee",        Konohagakure, Genin,        false));
    Registry.Add(CHAR_ENTRY("Character.Neji",           "Neji",           Konohagakure, Genin,        false));
    Registry.Add(CHAR_ENTRY("Character.Tenten",         "Tenten",         Konohagakure, Genin,        false));
    Registry.Add(CHAR_ENTRY("Character.Gai",            "Gai",            Konohagakure, Jonin,        false));
    Registry.Add(CHAR_ENTRY("Character.Shikamaru",      "Shikamaru",      Konohagakure, Chunin,       false));
    Registry.Add(CHAR_ENTRY("Character.Ino",            "Ino",            Konohagakure, Genin,        false));
    Registry.Add(CHAR_ENTRY("Character.Choji",          "Choji",          Konohagakure, Genin,        false));
    Registry.Add(CHAR_ENTRY("Character.Hinata",         "Hinata",         Konohagakure, Genin,        false));
    Registry.Add(CHAR_ENTRY("Character.Kiba",           "Kiba",           Konohagakure, Genin,        false));
    Registry.Add(CHAR_ENTRY("Character.Shino",          "Shino",          Konohagakure, Genin,        false));
    Registry.Add(CHAR_ENTRY("Character.Asuma",          "Asuma",          Konohagakure, Jonin,        false));
    Registry.Add(CHAR_ENTRY("Character.Kurenai",        "Kurenai",        Konohagakure, Jonin,        false));
    Registry.Add(CHAR_ENTRY("Character.Tsunade",        "Tsunade",        Konohagakure, Kage,         false));
    Registry.Add(CHAR_ENTRY("Character.Jiraiya",        "Jiraiya",        Konohagakure, Legendary,    false));
    Registry.Add(CHAR_ENTRY("Character.Orochimaru",     "Orochimaru",     Otogakure,    Legendary,    false));
    Registry.Add(CHAR_ENTRY("Character.Minato",         "Minato",         Konohagakure, Kage,         false));
    Registry.Add(CHAR_ENTRY("Character.Kushina",        "Kushina",        Konohagakure, Special_Jonin, false));
    Registry.Add(CHAR_ENTRY("Character.Hiruzen",        "Hiruzen",        Konohagakure, Kage,         false));
    Registry.Add(CHAR_ENTRY("Character.Itachi",         "Itachi",         Akatsuki,     ANBU,         false));
    Registry.Add(CHAR_ENTRY("Character.Shisui",         "Shisui",         Konohagakure, ANBU,         false));
    Registry.Add(CHAR_ENTRY("Character.Yamato",         "Yamato",         Konohagakure, ANBU,         false));
    Registry.Add(CHAR_ENTRY("Character.Sai",            "Sai",            Konohagakure, ANBU,         false));
    Registry.Add(CHAR_ENTRY("Character.Anko",           "Anko",           Konohagakure, Special_Jonin, false));
    Registry.Add(CHAR_ENTRY("Character.Ibiki",          "Ibiki",          Konohagakure, Special_Jonin, false));

    // ----------------------------------------------------------------
    //  HIDDEN SAND VILLAGE — Sunagakure
    // ----------------------------------------------------------------
    Registry.Add(CHAR_ENTRY("Character.Gaara",          "Gaara",          Sunagakure,   Genin,        false));
    Registry.Add(CHAR_ENTRY("Character.Temari",         "Temari",         Sunagakure,   Genin,        false));
    Registry.Add(CHAR_ENTRY("Character.Kankuro",        "Kankuro",        Sunagakure,   Genin,        false));
    Registry.Add(CHAR_ENTRY("Character.Chiyo",          "Chiyo",          Sunagakure,   Legendary,    false));
    Registry.Add(CHAR_ENTRY("Character.Rasa",           "Rasa",           Sunagakure,   Kage,         false));

    // ----------------------------------------------------------------
    //  HIDDEN MIST VILLAGE — Kirigakure
    // ----------------------------------------------------------------
    Registry.Add(CHAR_ENTRY("Character.Zabuza",         "Zabuza",         Kirigakure,   Special_Jonin, false));
    Registry.Add(CHAR_ENTRY("Character.Haku",           "Haku",           Kirigakure,   Genin,        false));
    Registry.Add(CHAR_ENTRY("Character.Kisame",         "Kisame",         Akatsuki,     Jonin,        false));
    Registry.Add(CHAR_ENTRY("Character.Mei",            "Mei",            Kirigakure,   Kage,         false));
    Registry.Add(CHAR_ENTRY("Character.Suigetsu",       "Suigetsu",       Kirigakure,   Genin,        false));
    Registry.Add(CHAR_ENTRY("Character.Jugo",           "Jugo",           Kirigakure,   Special_Jonin, false));
    Registry.Add(CHAR_ENTRY("Character.Karin",          "Karin",          Otogakure,    Genin,        false));

    // ----------------------------------------------------------------
    //  HIDDEN CLOUD VILLAGE — Kumogakure
    // ----------------------------------------------------------------
    Registry.Add(CHAR_ENTRY("Character.Killer_Bee",     "KillerBee",      Kumogakure,   Special_Jonin, false));
    Registry.Add(CHAR_ENTRY("Character.Raikage",        "Raikage",        Kumogakure,   Kage,         false));
    Registry.Add(CHAR_ENTRY("Character.Darui",          "Darui",          Kumogakure,   Special_Jonin, false));
    Registry.Add(CHAR_ENTRY("Character.Samui",          "Samui",          Kumogakure,   Jonin,        false));

    // ----------------------------------------------------------------
    //  HIDDEN STONE VILLAGE — Iwagakure
    // ----------------------------------------------------------------
    Registry.Add(CHAR_ENTRY("Character.Deidara",        "Deidara",        Akatsuki,     Jonin,        false));
    Registry.Add(CHAR_ENTRY("Character.Onoki",          "Onoki",          Iwagakure,    Kage,         false));
    Registry.Add(CHAR_ENTRY("Character.Kurotsuchi",     "Kurotsuchi",     Iwagakure,    Special_Jonin, false));

    // ----------------------------------------------------------------
    //  AKATSUKI
    // ----------------------------------------------------------------
    Registry.Add(CHAR_ENTRY("Character.Pain_Nagato",    "PainNagato",     Akatsuki,     Legendary,    false));
    Registry.Add(CHAR_ENTRY("Character.Konan",          "Konan",          Akatsuki,     Jonin,        false));
    Registry.Add(CHAR_ENTRY("Character.Tobi_Obito",     "TobiObito",      Akatsuki,     ANBU,         false));
    Registry.Add(CHAR_ENTRY("Character.Madara",         "Madara",         Akatsuki,     Legendary,    false));
    Registry.Add(CHAR_ENTRY("Character.Sasori",         "Sasori",         Akatsuki,     Legendary,    false));
    Registry.Add(CHAR_ENTRY("Character.Hidan",          "Hidan",          Akatsuki,     Jonin,        false));
    Registry.Add(CHAR_ENTRY("Character.Kakuzu",         "Kakuzu",         Akatsuki,     Jonin,        false));
    Registry.Add(CHAR_ENTRY("Character.Zetsu_White",    "ZetsuWhite",     Akatsuki,     Special_Jonin, false));
    Registry.Add(CHAR_ENTRY("Character.Zetsu_Black",    "ZetsuBlack",     Akatsuki,     Special_Jonin, false));

    // ----------------------------------------------------------------
    //  OTSUTSUKI CLAN
    // ----------------------------------------------------------------
    Registry.Add(CHAR_ENTRY("Character.Kaguya",         "Kaguya",         Otsutsuki,    Otsutsuki,    false));
    Registry.Add(CHAR_ENTRY("Character.Momoshiki",      "Momoshiki",      Otsutsuki,    Otsutsuki,    false));
    Registry.Add(CHAR_ENTRY("Character.Kinshiki",       "Kinshiki",       Otsutsuki,    Otsutsuki,    false));
    Registry.Add(CHAR_ENTRY("Character.Hamura",         "Hamura",         Otsutsuki,    Otsutsuki,    false));
    Registry.Add(CHAR_ENTRY("Character.Hagoromo",       "Hagoromo",       Otsutsuki,    Otsutsuki,    false));

    // ----------------------------------------------------------------
    //  NARUTO SHIPPUDEN — Additional characters
    // ----------------------------------------------------------------
    Registry.Add(CHAR_ENTRY("Character.Naruto_Sage",    "NarutoSage",     Konohagakure, Jonin,        false));
    Registry.Add(CHAR_ENTRY("Character.Naruto_KCM",     "NarutoKCM",      Konohagakure, Legendary,    false));
    Registry.Add(CHAR_ENTRY("Character.Naruto_SPSM",    "NarutoSPSM",     Konohagakure, Legendary,    false));
    Registry.Add(CHAR_ENTRY("Character.Sasuke_CS2",     "SasukeCS2",      Akatsuki,     Jonin,        false));
    Registry.Add(CHAR_ENTRY("Character.Sasuke_Rinnegan","SasukeRinnegan", Konohagakure, Legendary,    false));
    Registry.Add(CHAR_ENTRY("Character.Kakashi_DMS",    "KakashiDMS",     Konohagakure, Legendary,    false));
    Registry.Add(CHAR_ENTRY("Character.Sakura_Adult",   "SakuraAdult",    Konohagakure, Special_Jonin, false));
    Registry.Add(CHAR_ENTRY("Character.Gaara_Kage",     "GaaraKage",      Sunagakure,   Kage,         false));
    Registry.Add(CHAR_ENTRY("Character.Rock_Lee_Gates", "RockLeeGates",   Konohagakure, Jonin,        false));
    Registry.Add(CHAR_ENTRY("Character.Neji_Byakugan",  "NejiByakugan",   Konohagakure, Special_Jonin, false));
    Registry.Add(CHAR_ENTRY("Character.Shikamaru_Adult","ShikamaruAdult", Konohagakure, Jonin,        false));
    Registry.Add(CHAR_ENTRY("Character.Hinata_Adult",   "HinataAdult",    Konohagakure, Jonin,        false));
    Registry.Add(CHAR_ENTRY("Character.Choji_Butterfly","ChojiButterfly", Konohagakure, Jonin,        false));
    Registry.Add(CHAR_ENTRY("Character.Senjuu_Hashirama","HashiramaFirst",Konohagakure, Legendary,    false));
    Registry.Add(CHAR_ENTRY("Character.Senjuu_Tobirama","TobiramaSecond",Konohagakure, Legendary,    false));
    Registry.Add(CHAR_ENTRY("Character.Hiruzen_Prime",  "HiruzenPrime",   Konohagakure, Legendary,    false));
    Registry.Add(CHAR_ENTRY("Character.Minato_KCM",     "MinatoKCM",      Konohagakure, Legendary,    false));
    Registry.Add(CHAR_ENTRY("Character.Might_Gai_8G",   "MightGai8G",     Konohagakure, Legendary,    false));

    // ----------------------------------------------------------------
    //  SUPPORTING / SIDE CHARACTERS
    // ----------------------------------------------------------------
    Registry.Add(CHAR_ENTRY("Character.Kabuto",         "Kabuto",         Otogakure,    Special_Jonin, false));
    Registry.Add(CHAR_ENTRY("Character.Kabuto_Sage",    "KabutoSage",     Otogakure,    Legendary,    false));
    Registry.Add(CHAR_ENTRY("Character.Danzo",          "Danzo",          Konohagakure, Jonin,        false));
    Registry.Add(CHAR_ENTRY("Character.Fu_ANBU",        "FuANBU",         Konohagakure, ANBU,         false));
    Registry.Add(CHAR_ENTRY("Character.Torune",         "Torune",         Konohagakure, ANBU,         false));
    Registry.Add(CHAR_ENTRY("Character.Konohamaru",     "Konohamaru",     Konohagakure, Genin,        false));
    Registry.Add(CHAR_ENTRY("Character.Iruka",          "Iruka",          Konohagakure, Chunin,       false));

    // ----------------------------------------------------------------
    //  TRANSFORMATIONS (treated as separate selectable entries)
    // ----------------------------------------------------------------
    Registry.Add(CHAR_ENTRY("Character.Kurama_Full",    "KuramaFull",     Konohagakure, Legendary,    false));
    Registry.Add(CHAR_ENTRY("Character.Ten_Tails",      "TenTails",       Otsutsuki,    Otsutsuki,    false));
    Registry.Add(CHAR_ENTRY("Character.Juubi_Jinchuuriki","JuubiJin",    Otsutsuki,    Otsutsuki,    false));

    // ----------------------------------------------------------------
    //  PLACEHOLDER SLOTS — remaining 60 entries to reach 150 total
    //  Replace these with real characters as they are designed.
    // ----------------------------------------------------------------
    const TArray<TPair<FString,EVillage>> PlaceholderChars =
    {
        { TEXT("Yugito"),       EVillage::Kumogakure },
        { TEXT("Yagura"),       EVillage::Kirigakure },
        { TEXT("Roshi"),        EVillage::Iwagakure  },
        { TEXT("Han"),          EVillage::Iwagakure  },
        { TEXT("Utakata"),      EVillage::Kirigakure },
        { TEXT("Fuu"),          EVillage::None        },
        { TEXT("Chomei"),       EVillage::None        },
        { TEXT("Isobu"),        EVillage::None        },
        { TEXT("Son_Goku"),     EVillage::None        },
        { TEXT("Kokuo"),        EVillage::None        },
        { TEXT("Saiken"),       EVillage::None        },
        { TEXT("Matatabi"),     EVillage::None        },
        { TEXT("Pakura"),       EVillage::Sunagakure  },
        { TEXT("Gari"),         EVillage::Iwagakure  },
        { TEXT("Mifune"),       EVillage::None        },
        { TEXT("Darui_Raikage"),EVillage::Kumogakure  },
        { TEXT("C_Cloud"),      EVillage::Kumogakure  },
        { TEXT("Omoi"),         EVillage::Kumogakure  },
        { TEXT("Karui"),        EVillage::Kumogakure  },
        { TEXT("Chojuro"),      EVillage::Kirigakure  },
        { TEXT("Ao"),           EVillage::Kirigakure  },
        { TEXT("Mangetsu"),     EVillage::Kirigakure  },
        { TEXT("Fuguki"),       EVillage::Kirigakure  },
        { TEXT("Kushimaru"),    EVillage::Kirigakure  },
        { TEXT("Jinin"),        EVillage::Kirigakure  },
        { TEXT("Jinpachi"),     EVillage::Kirigakure  },
        { TEXT("Ameyuri"),      EVillage::Kirigakure  },
        { TEXT("Raiga"),        EVillage::Kirigakure  },
        { TEXT("Pakura_S"),     EVillage::Sunagakure  },
        { TEXT("Ebizo"),        EVillage::Sunagakure  },
        { TEXT("Sasori_Hiruko"),EVillage::Akatsuki    },
        { TEXT("Kizashi"),      EVillage::Konohagakure},
        { TEXT("Mebuki"),       EVillage::Konohagakure},
        { TEXT("Inoichi"),      EVillage::Konohagakure},
        { TEXT("Shikaku"),      EVillage::Konohagakure},
        { TEXT("Choza"),        EVillage::Konohagakure},
        { TEXT("Hiashi"),       EVillage::Konohagakure},
        { TEXT("Hizashi"),      EVillage::Konohagakure},
        { TEXT("Nawaki"),       EVillage::Konohagakure},
        { TEXT("Dan_Kato"),     EVillage::Konohagakure},
        { TEXT("Fugaku"),       EVillage::Konohagakure},
        { TEXT("Mikoto"),       EVillage::Konohagakure},
        { TEXT("Obito_Young"),  EVillage::Konohagakure},
        { TEXT("Rin"),          EVillage::Konohagakure},
        { TEXT("Kakashi_Young"),EVillage::Konohagakure},
        { TEXT("Yahiko"),       EVillage::Amegakure  },
        { TEXT("Nagato_Young"), EVillage::Amegakure  },
        { TEXT("Konan_Young"),  EVillage::Amegakure  },
        { TEXT("Deva_Path"),    EVillage::Akatsuki    },
        { TEXT("Asura_Path"),   EVillage::Akatsuki    },
        { TEXT("Human_Path"),   EVillage::Akatsuki    },
        { TEXT("Animal_Path"),  EVillage::Akatsuki    },
        { TEXT("Preta_Path"),   EVillage::Akatsuki    },
        { TEXT("Naraka_Path"),  EVillage::Akatsuki    },
        { TEXT("Fuu_Jinchuuriki"), EVillage::None     },
        { TEXT("Killer_Bee_V2"),   EVillage::Kumogakure},
        { TEXT("Naruto_Baryon"),   EVillage::Konohagakure},
        { TEXT("Sasuke_Adult"),    EVillage::Konohagakure},
        { TEXT("Boruto"),          EVillage::Konohagakure},
        { TEXT("Sarada"),          EVillage::Konohagakure},
    };

    for (const auto& Pair : PlaceholderChars)
    {
        const FString& Name = Pair.Key;
        const EVillage  Village = Pair.Value;

        FCharacterRegistryEntry Entry;
        Entry.CharacterTag = FGameplayTag::RequestGameplayTag(
            FName(*FString::Printf(TEXT("Character.%s"), *Name)), false);
        Entry.DisplayName        = FText::FromString(Name.Replace(TEXT("_"), TEXT(" ")));
        Entry.CharacterDataAsset = FSoftObjectPath(
            FString::Printf(TEXT("/Game/Characters/%s/DA_Character_%s.DA_Character_%s"),
                *Name, *Name, *Name));
        Entry.PortraitTexture    = FSoftObjectPath(
            FString::Printf(TEXT("/Game/Characters/%s/Textures/T_%s_Portrait.T_%s_Portrait"),
                *Name, *Name, *Name));
        Entry.FullBodyTexture    = FSoftObjectPath(
            FString::Printf(TEXT("/Game/Characters/%s/Textures/T_%s_FullBody.T_%s_FullBody"),
                *Name, *Name, *Name));
        Entry.FPArmsSkeletalMesh = FSoftObjectPath(
            FString::Printf(TEXT("/Game/Characters/%s/Meshes/SK_%s_FPArms.SK_%s_FPArms"),
                *Name, *Name, *Name));
        Entry.CharacterSkeletalMesh = FSoftObjectPath(
            FString::Printf(TEXT("/Game/Characters/%s/Meshes/SK_%s.SK_%s"),
                *Name, *Name, *Name));
        Entry.AnimBlueprint      = FSoftObjectPath(
            FString::Printf(TEXT("/Game/Characters/%s/ABP_%s.ABP_%s"),
                *Name, *Name, *Name));
        Entry.Village            = Village;
        Entry.Rank               = EShinobi_Rank::Genin;
        Entry.bUnlocked          = false;
        Entry.bIsPlaceholder     = true;

        Registry.Add(Entry);
    }

    return Registry;
}

#undef CHAR_ENTRY
