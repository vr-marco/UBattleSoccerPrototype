

using UnrealBuildTool;
using System.Collections.Generic;

public class MagicBattleSoccerTarget : TargetRules
{
	public MagicBattleSoccerTarget(TargetInfo Target) : base(Target)
	{
        DefaultBuildSettings = BuildSettingsVersion.V2;

        Type = TargetType.Game;
        ExtraModuleNames.AddRange(new string[] { "MagicBattleSoccer" });
    }
}
