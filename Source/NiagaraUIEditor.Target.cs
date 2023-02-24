// Copyright 2023 - Michal Smoleň

using UnrealBuildTool;
using System.Collections.Generic;

public class NiagaraUIEditorTarget : TargetRules
{
	public NiagaraUIEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( new string[] { "NiagaraUI" } );
	}
}
