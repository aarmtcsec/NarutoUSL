# Naruto: Ultimate Shinobi Legacy
### AAA Open World Action RPG — Unreal Engine 5.6

---

## How to Run on Windows

### Step 1 — Install Unreal Engine 5.6

1. Download the **Epic Games Launcher**: [epicgames.com/store/en-US/download](https://www.epicgames.com/store/en-US/download)
2. Install and open it
3. Click the **Unreal Engine** tab on the left
4. Click **Library** at the top
5. Click the **+** button next to Engine Versions
6. Select **5.6** from the dropdown
7. Click **Install** — choose a drive with at least 60GB free
8. Wait for it to finish (takes 30–60 minutes)

---

### Step 2 — Install Visual Studio 2022

1. Download from: [visualstudio.microsoft.com/downloads](https://visualstudio.microsoft.com/downloads)
2. Choose **Community** (free)
3. Run the installer
4. On the workloads screen, check these two:
   - **Desktop development with C++**
   - **Game development with C++**
5. On the right side under Individual Components, also check:
   - **.NET Framework 4.6.2 targeting pack**
   - **Windows 10/11 SDK**
6. Click **Install** — takes 20–40 minutes

---

### Step 3 — Get the Project

**Option A — Clone from GitHub (recommended):**

1. Install Git from [git-scm.com/download/win](https://git-scm.com/download/win) — click through the installer with default options
2. Open **Command Prompt** or **PowerShell**
3. Run:
```
git clone https://github.com/aarmtcsec/NarutoUSL.git
```
4. Wait for it to download

**Option B — Download ZIP:**

1. Go to [github.com/aarmtcsec/NarutoUSL](https://github.com/aarmtcsec/NarutoUSL)
2. Click the green **Code** button
3. Click **Download ZIP**
4. Extract it somewhere on your PC (e.g. `C:\Projects\NarutoUSL`)

---

### Step 4 — Generate Visual Studio Project Files

1. Find the file `NarutoUSL.uproject` in the project folder
2. Right-click it
3. Click **Generate Visual Studio project files**
4. Wait a few seconds — it creates a `NarutoUSL.sln` file in the same folder

If you don't see that option when you right-click, open Command Prompt and run:
```
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\GenerateProjectFiles.bat" "C:\path\to\NarutoUSL\NarutoUSL.uproject" -game
```
Replace `C:\path\to\NarutoUSL` with wherever you put the project.

---

### Step 5 — Build the C++ Code

1. Double-click `NarutoUSL.sln` to open it in Visual Studio 2022
2. At the top of Visual Studio, change the configuration dropdowns to:
   - **Development Editor**
   - **Win64**
3. In the Solution Explorer on the right, right-click **NarutoUSL**
4. Click **Build**
5. Wait — first build takes 10–20 minutes
6. You should see **Build succeeded** at the bottom

---

### Step 6 — Open in Unreal Editor

1. Double-click `NarutoUSL.uproject`
2. If it asks which engine version to use, select **5.6**
3. The Unreal Editor opens and compiles shaders — takes a few minutes the first time
4. You are now in the editor

---

### Step 7 — Make it Playable

The C++ framework is complete but you need a few things set up in the editor before hitting Play:

| What | How |
|---|---|
| Create a test level | File → New Level → Basic |
| Add navigation | Place Actors panel → search **Nav Mesh Bounds Volume** → drag into the level → scale it to cover the floor |
| Create player Blueprint | Content Browser → right-click → Blueprint Class → search and select `NarutoPlayerCharacter` → name it `BP_NarutoPlayerCharacter` |
| Set as default pawn | Edit → Project Settings → Maps & Modes → Default Pawn Class → select `BP_NarutoPlayerCharacter` |
| Import placeholder textures | In Content Browser → right-click `Content/Characters` → Import → select all PNG files → Import All |
| Hit Play | Press the green **Play** button in the toolbar |

---

### Troubleshooting

**"Could not be compiled. Try rebuilding from source."**
Open the `.sln` in Visual Studio and build again. Check the Output window for the actual error.

**"NarutoUSL.uproject is not associated with UE5"**
Right-click the `.uproject` → Open With → select Unreal Engine 5.6.

**Visual Studio not found when right-clicking .uproject**
Make sure Visual Studio 2022 is installed with the **Game development with C++** workload. Reinstall that workload from the Visual Studio Installer if needed.

**Build errors about missing modules**
Make sure you installed UE5.6 specifically — not 5.4 or 5.5. The Build.cs targets 5.6.

**Shader compilation takes forever**
Normal on first run. Let it finish. Subsequent opens are much faster.

---

## How to Run on Mac

### Requirements
- **Unreal Engine 5.6** via Epic Games Launcher
- **Xcode 15+** from the Mac App Store

### Steps

1. Install Xcode from the App Store
2. Open Terminal and run:
```
xcode-select --install
sudo xcodebuild -license accept
```
3. Right-click `NarutoUSL.uproject` → **Generate Xcode Project**
4. Open the generated `.xcworkspace` in Xcode
5. Set scheme to **NarutoUSL**, configuration to **Development Editor**
6. Press **Cmd+B** to build
7. Double-click `NarutoUSL.uproject` to open in the editor

---

## Project Structure

```
NarutoUSL/
├── Config/                  — Engine, game, input configuration
├── Content/
│   └── Characters/          — 218 placeholder PNG textures (109 characters)
├── Source/NarutoUSL/
│   ├── AI/                  — AIDirector, BossPhaseManager
│   ├── Analytics/           — Telemetry, performance tracking
│   ├── Audio/               — AudioManager (music, SFX, voice)
│   ├── Chakra/              — ChakraSystem subsystem
│   ├── Character/
│   │   ├── Base/            — NarutoCharacterBase (root for all characters)
│   │   ├── Boss/            — NarutoBossBase + Boss_Pain implementation
│   │   ├── Components/      — All 9 character components
│   │   ├── Data/            — NarutoCharacterData asset + CharacterRegistry
│   │   ├── Enemy/           — NarutoEnemyBase
│   │   └── Player/          — PlayerCharacter (first-person), PlayerController, PlayerState
│   ├── Combat/
│   │   ├── Combos/          — ComboGraph data asset + ComboSystem
│   │   ├── Core/            — CombatManager, HitboxManager
│   │   ├── Counters/        — SubstitutionSystem, ParrySystem
│   │   └── Damage/          — DamageCalculator, DamageTypes
│   ├── Core/
│   │   ├── Events/          — NarutoEventBus
│   │   ├── GameInstance/    — NarutoGameInstance (owns all 15 subsystems)
│   │   ├── GameMode/        — NarutoGameMode
│   │   ├── GameState/       — NarutoGameState
│   │   ├── Interfaces/      — ICombatant, IChakraUser, IDamageable, etc.
│   │   ├── Settings/        — NarutoGameSettings
│   │   ├── Subsystems/      — NarutoSubsystem base class
│   │   └── Types/           — All enums and structs
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
├── Tools/
│   └── GeneratePlaceholderTextures.py   — regenerates placeholder art
├── .gitignore
└── NarutoUSL.uproject
```

---

## System Summary

| System | Key Capability |
|---|---|
| Core | GameInstance owns 15 subsystems in dependency order |
| Character | First-person camera, 9 components, all 5 interfaces |
| Combat | 11-stage damage pipeline, frame data, hitboxes, substitution, parry |
| Jutsu | 6 jutsu types, hand seals, mastery scaling, sustain, charge |
| AI | Threat tables, encounter budget, adaptive difficulty, boss phases |
| World | Day/night, seasons, weather blending, streaming |
| Narrative | Quest states, dialogue trees, world flags, faction reputation |
| Economy | Currency, loot rolling, shop with reputation discounts |
| Save | Versioned saves, migration pipeline, autosave, cloud hooks |
| Audio | Layered dynamic music, voice priority queue |
| UI | Screen stack, cinematic mode, first-person HUD |

---

## Syncing Changes (Git)

Push updates:
```
git add .
git commit -m "describe your change"
git push
```

Pull updates on another computer:
```
git pull
```
