

using UnrealBuildTool;
using System.Collections.Generic;

public class MagicBattleSoccerEditorTarget : TargetRules
{
	public MagicBattleSoccerEditorTarget(TargetInfo Target): base(Target)
	{
        DefaultBuildSettings = BuildSettingsVersion.V2;

        Type = TargetType.Editor;
        ExtraModuleNames.AddRange(new string[] { "MagicBattleSoccer" });
    }
}
