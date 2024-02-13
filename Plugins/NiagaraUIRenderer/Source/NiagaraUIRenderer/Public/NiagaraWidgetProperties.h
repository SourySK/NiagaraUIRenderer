﻿// Copyright 2022 - Michal Smoleň

#pragma once

class UMaterialInterface;

struct FNiagaraWidgetProperties
{
	FNiagaraWidgetProperties();
	FNiagaraWidgetProperties(TMap<UMaterialInterface*, UMaterialInterface*>* inRemapList, bool inAutoActivate, bool inShowDebugSystem, bool inPassDynamicParametersFromRibbon, bool inFakeDepthScale, float inFakeDepthDistance)
		: MaterialRemapList(inRemapList), AutoActivate(inAutoActivate), ShowDebugSystemInWorld(inShowDebugSystem), PassDynamicParametersFromRibbon(inPassDynamicParametersFromRibbon), FakeDepthScale(inFakeDepthScale), FakeDepthScaleDistance(inFakeDepthDistance) {}
	
	TMap<UMaterialInterface*, UMaterialInterface*>* MaterialRemapList = nullptr;
	bool AutoActivate = true;
	bool ShowDebugSystemInWorld = false;
	bool PassDynamicParametersFromRibbon = false;
	bool FakeDepthScale = false;
	float FakeDepthScaleDistance = 1000.f;
};
