using UnrealBuildTool;

public class SornprommasTarget : TargetRules
{
	public SornprommasTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Sornprommas");
	}
}
