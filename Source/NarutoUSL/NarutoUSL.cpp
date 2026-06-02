// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "NarutoUSL.h"
#include "Modules/ModuleManager.h"

// Log category definitions
DEFINE_LOG_CATEGORY(LogNarutoUSL);
DEFINE_LOG_CATEGORY(LogNarutoCombat);
DEFINE_LOG_CATEGORY(LogNarutoChakra);
DEFINE_LOG_CATEGORY(LogNarutoJutsu);
DEFINE_LOG_CATEGORY(LogNarutoAI);
DEFINE_LOG_CATEGORY(LogNarutoNarrative);
DEFINE_LOG_CATEGORY(LogNarutoSave);
DEFINE_LOG_CATEGORY(LogNarutoWorld);
DEFINE_LOG_CATEGORY(LogNarutoUI);
DEFINE_LOG_CATEGORY(LogNarutoAudio);

class FNarutoUSLModule : public IModuleInterface
{
public:

    virtual void StartupModule() override
    {
        UE_LOG(LogNarutoUSL, Log, TEXT("NarutoUSL module started."));
    }

    virtual void ShutdownModule() override
    {
        UE_LOG(LogNarutoUSL, Log, TEXT("NarutoUSL module shut down."));
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(FNarutoUSLModule, NarutoUSL, "NarutoUSL");
