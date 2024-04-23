// Copyright 2023 - Michal Smoleň

#pragma once

#include "CoreMinimal.h"

class UMaterialInterface;

struct FNiagaraWidgetProperties
{
	FNiagaraWidgetProperties();
	FNiagaraWidgetProperties(TMap<TObjectPtr<UMaterialInterface>, UMaterialInterface*>* inRemapList, bool inAutoActivate, bool inShowDebugSystem, bool inPassDynamicParametersFromRibbon, bool inFakeDepthScale, float inFakeDepthDistance)
        : MaterialRemapList(inRemapList), AutoActivate(inAutoActivate), ShowDebugSystemInWorld(inShowDebugSystem), PassDynamicParametersFromRibbon(inPassDynamicParametersFromRibbon), FakeDepthScale(inFakeDepthScale), FakeDepthScaleDistance(inFakeDepthDistance) {}
	
	TMap<TObjectPtr<UMaterialInterface>, UMaterialInterface*>* MaterialRemapList = nullptr;
	bool AutoActivate = true;
	bool ShowDebugSystemInWorld = false;
	bool PassDynamicParametersFromRibbon = false;
	bool FakeDepthScale = false;
	float FakeDepthScaleDistance = 1000.f;
};
