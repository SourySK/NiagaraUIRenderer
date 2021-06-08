// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Actor.h"
#include "NiagaraUiActor.generated.h"

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