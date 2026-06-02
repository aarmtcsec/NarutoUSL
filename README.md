# Naruto: Ultimate Shinobi Legacy
### AAA Open World Action RPG — Unreal Engine 5.6

---

## How to Open This Project

### Requirements
- **Unreal Engine 5.6** — [Download from Epic Games Launcher](https://www.unrealengine.com)
- **Visual Studio 2022** (Windows) with these workloads:
  - Desktop development with C++
  - Game development with C++
- **Xcode 15+** (macOS) — for Mac compilation

### Steps

1. **Generate project files**
   Right-click `NarutoUSL.uproject` → **Generate Xcode Project** (Mac) or **Generate Visual Studio files** (Windows)

2. **Build the project**
   Open the generated `.xcworkspace` (Mac) or `.sln` (Windows) and build the `NarutoUSL` target in **Development Editor** configuration.

3. **Open in Unreal Editor**
   Double-click `NarutoUSL.uproject` — UE5 will open and compile automatically.

4. **First-time setup in editor**
   - Open **Project Settings → Asset Manager** and verify the data asset scan paths
   - Open **Project Settings → GameplayTags** and create the gameplay tags table at `/Game/Data/Tags/NarutoGameplayTags`
   - Create an **Input Mapping Context** asset and assign it to `BP_NarutoPlayerCharacter.DefaultMappingContext`

5. **Play in Editor**
   Press the green **Play** button in the UE5 toolbar.

---

## Project Structure

```
NarutoUSL/
├── Config/                  — Engine, game, input configuration
├── Source/NarutoUSL/
│   ├── AI/                  — AIDirector, BossPhaseManager
│   ├── Analytics/           — Telemetry, performance tracking
│   ├── Audio/               — AudioManager (music, SFX, voice)
│   ├── Chakra/              — ChakraSystem subsystem
│   ├── Character/
│   │   ├── Base/            — NarutoCharacterBase (root for all characters)
│   │   ├── Boss/            — NarutoBossBase + Boss_Pain implementation
│   │   ├── Components/      — All 9 character components
│   │   ├── Data/            — NarutoCharacterData asset
│   │   ├── Enemy/           — NarutoEnemyBase
│   │   └── Player/          — PlayerCharacter, PlayerController, PlayerState
│   ├── Combat/
│   │   ├── Combos/          — ComboGraph data asset + ComboSystem
│   │   ├── Core/            — CombatManager, HitboxManager
│   │   ├── Counters/        — SubstitutionSystem, ParrySystem
│   │   └── Damage/          — DamageCalculator, DamageTypes
│   ├── Core/
│   │   ├── Events/          — NarutoEventBus
│   │   ├── GameInstance/    — NarutoGameInstance
│   │   ├── GameMode/        — NarutoGameMode
│   │   ├── GameState/       — NarutoGameState
│   │   ├── Interfaces/      — ICombatant, IChakraUser, IDamageable, etc.
│   │   ├── Settings/        — NarutoGameSettings (DeveloperSettings)
│   │   ├── Subsystems/      — NarutoSubsystem base class
│   │   └── Types/           — NarutoTypes (all enums and structs)
│   ├── Economy/             — EconomyManager (Ryo, loot, shop)
│   ├── Jutsu/
│   │   ├── Core/            — JutsuManager, JutsuExecutor, HandSealSystem
│   │   ├── Data/            — JutsuData asset
│   │   └── Types/           — 6 jutsu type bases
│   ├── Narrative/
│   │   ├── Core/            — NarrativeManager, WorldStateManager
│   │   ├── Faction/         — FactionManager
│   │   └── Quest/           — QuestManager
│   ├── Progression/         — ProgressionManager, SkillTree
│   ├── Save/                — SaveManager (versioned, cloud, backup)
│   ├── UI/                  — UIManager, screen stack
│   └── World/               — WorldManager, StreamingManager
└── NarutoUSL.uproject
```

---

## System Summary

| System | Files | Key Capability |
|---|---|---|
| Core Architecture | 12 | GameInstance owns 15 subsystems in dependency order |
| Character Base | 13 | All components, all 5 interfaces implemented |
| Combat Framework | 14 | 11-stage damage pipeline, frame data, hitboxes, substitution, parry |
| Jutsu Pipeline | 14 | 6 jutsu types, hand seals, mastery scaling, sustain, charge |
| AI Framework | 4 | Threat tables, encounter budget, adaptive difficulty, boss phases |
| World Systems | 4 | Day/night, seasons, weather blending, streaming |
| Narrative | 6 | Quest states, dialogue trees, world flags, faction reputation |
| Economy | 2 | Currency, loot rolling, shop pricing with reputation discounts |
| Progression | 2 | XP, levels, skill tree, playtime tracking |
| Save System | 2 | Versioned saves, migration pipeline, autosave, cloud hooks |
| Audio | 2 | Layered music, voice priority queue, dynamic ambience |
| UI | 2 | Screen stack, cinematic mode, damage numbers |

**Total: 117 C++ files across 122 project files.**

---

## Next Steps to Make it Playable

1. Create `BP_NarutoPlayerCharacter` Blueprint subclassing `NarutoPlayerCharacter`
2. Create `DA_Character_Naruto` data asset using `NarutoCharacterData`
3. Create `DA_Jutsu_Rasengan` data asset using `JutsuData`
4. Create `DA_ComboGraph_Naruto` data asset using `ComboGraphData`
5. Create a test level with a `NavMeshBoundsVolume`
6. Place a `BP_NarutoPlayerCharacter` as the default pawn
7. Assign the character data asset and combo graph
8. Press Play
