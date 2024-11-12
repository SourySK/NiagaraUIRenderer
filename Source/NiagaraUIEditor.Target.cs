// Copyright 2024 - Michal Smoleň

using UnrealBuildTool;
using System.Collections.Generic;

public class NiagaraUIEditorTarget : TargetRules
{
	public NiagaraUIEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.AddRange( new string[] { "NiagaraUI" } );
	}
}
