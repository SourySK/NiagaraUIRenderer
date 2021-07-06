// Copyright 2021 - Michal Smoleň

#pragma once

#include "CoreMinimal.h"
#include "NiagaraUIActor.generated.h"

class UNiagaraSystem;
class UNiagaraUIComponent;

UCLASS()
class ANiagaraUIActor : public AActor
{
	GENERATED_BODY()

public:
	ANiagaraUIActor();
	
    class UNiagaraUIComponent* SpawnNewNiagaraUIComponent(UNiagaraSystem* NiagaraSystemTemplate, bool AutoActivate, bool ShowDebugSystem, bool TickWhenPaused);
};