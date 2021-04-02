// Fill out your copyright notice in the Description page of Project Settings.


#include "NiagaraUIActor.h"
#include "NiagaraUIComponent.h"
#include "Components/SceneComponent.h"

PRAGMA_DISABLE_OPTIMIZATION

ANiagaraUIActor::ANiagaraUIActor()
{
	if (!RootComponent)
		RootComponent = CreateDefaultSubobject<USceneComponent>(USceneComponent::GetDefaultSceneRootVariableName());
}

UNiagaraUIComponent* ANiagaraUIActor::SpawnNewNiagaraUIComponent(UNiagaraSystem* NiagaraSystemTemplate, bool AutoActivate, bool ShowDebugSystem)
{
	UNiagaraUIComponent* newComponent = NewObject<UNiagaraUIComponent>(this);

	newComponent->SetupAttachment(RootComponent);
	newComponent->SetAutoActivate(AutoActivate);
	newComponent->SetHiddenInGame(!ShowDebugSystem);
	newComponent->RegisterComponent();
	newComponent->SetAsset(NiagaraSystemTemplate);
	newComponent->SetAutoDestroy(false);

	return newComponent;
}
PRAGMA_ENABLE_OPTIMIZATION