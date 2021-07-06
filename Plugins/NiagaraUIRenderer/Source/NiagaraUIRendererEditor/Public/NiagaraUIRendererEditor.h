// Copyright 2021 - Michal Smoleň

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FNiagaraUIRendererEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
