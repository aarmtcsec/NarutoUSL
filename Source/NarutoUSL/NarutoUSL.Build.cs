// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// Module Build Configuration

using UnrealBuildTool;
using System.Collections.Generic;

public class NarutoUSL : ModuleRules
{
    public NarutoUSL(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = false;
        CppStandard = CppStandardVersion.Cpp20;

        // Optimize for shipping builds
        if (Target.Configuration == UnrealTargetConfiguration.Shipping)
        {
            bAllowConfidentialPlatformDefines = true;
            OptimizeCode = CodeOptimization.InNonDebugBuilds;
        }

        PublicIncludePaths.AddRange(new string[]
        {
            "NarutoUSL/Public",
        });

        PrivateIncludePaths.AddRange(new string[]
        {
            "NarutoUSL/Private",
        });

        // Core engine dependencies
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "AIModule",
            "NavigationSystem",
            "PhysicsCore",
            "Chaos",
            "ChaosSolverEngine",
            "GeometryCollectionEngine",
            "FieldSystemEngine",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            // Animation
            "AnimationCore",
            "AnimGraphRuntime",
            "MotionWarping",
            "PoseSearch",
            "ContextualAnimation",

            // Audio
            "AudioMixer",
            "AudioExtensions",
            "MetasoundEngine",
            "MetasoundFrontend",
            "SignalProcessing",

            // Rendering
            "Renderer",
            "RenderCore",
            "RHI",
            "Niagara",
            "NiagaraCore",

            // UI
            "UMG",
            "Slate",
            "SlateCore",
            "CommonUI",
            "CommonInput",
            "CommonGame",

            // World / Streaming
            "WorldPartitionEditor",
            "LandscapeEditor",
            "Foliage",

            // Networking
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "NetCore",

            // Save / Serialization
            "Json",
            "JsonUtilities",

            // Platform
            "ApplicationCore",
            "Projects",

            // Developer tools (editor only)
            "DeveloperSettings",
        });

        // Editor-only modules
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "EditorFramework",
                "LevelEditor",
                "PropertyEditor",
                "EditorStyle",
                "AssetTools",
                "AssetRegistry",
                "BlueprintGraph",
                "KismetCompiler",
                "GraphEditor",
                "ToolMenus",
                "ContentBrowser",
                "SequencerWidgets",
            });
        }

        // Platform-specific dependencies
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("NARUTOUSL_PLATFORM_PC=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.XboxOne ||
                 Target.Platform == UnrealTargetPlatform.XSX)
        {
            PublicDefinitions.Add("NARUTOUSL_PLATFORM_XBOX=1");
            PrivateDependencyModuleNames.Add("XboxCommon");
        }
        else if (Target.Platform == UnrealTargetPlatform.PS4 ||
                 Target.Platform == UnrealTargetPlatform.PS5)
        {
            PublicDefinitions.Add("NARUTOUSL_PLATFORM_PLAYSTATION=1");
        }

        // Global compile definitions
        PublicDefinitions.AddRange(new string[]
        {
            "NARUTOUSL_VERSION_MAJOR=1",
            "NARUTOUSL_VERSION_MINOR=0",
            "NARUTOUSL_VERSION_PATCH=0",
            "NARUTOUSL_MAX_PLAYERS=4",
            "NARUTOUSL_MAX_CHARACTERS=150",
            "NARUTOUSL_MAX_JUTSU=1000",
            "NARUTOUSL_MAX_ACTIVE_ENEMIES=64",
        });

        // Enable RTTI for reflection-heavy systems
        bEnableExceptions = true;
        bEnableUndefinedIdentifierWarnings = false;
    }
}
