// Copyright 2021 - Michal Smoleň

#include "NiagaraUIActor.h"
#include "NiagaraUIComponent.h"
#include "Components/SceneComponent.h"

PRAGMA_DISABLE_OPTIMIZATION

ANiagaraUIActor::ANiagaraUIActor()
{
	if (!RootComponent)
		RootComponent = CreateDefaultSubobject<USceneComponent>(USceneComponent::GetDefaultSceneRootVariableName());
}

UNiagaraUIComponent* ANiagaraUIActor::SpawnNewNiagaraUIComponent(UNiagaraSystem* NiagaraSystemTemplate, bool AutoActivate, bool ShowDebugSystem, bool TickWhenPaused)
{
	UNiagaraUIComponent* newComponent = NewObject<UNiagaraUIComponent>(this);

	newComponent->SetupAttachment(RootComponent);
	newComponent->SetAutoActivate(AutoActivate);
	newComponent->SetHiddenInGame(!ShowDebugSystem);
	newComponent->RegisterComponent();
	newComponent->SetAsset(NiagaraSystemTemplate);
	newComponent->SetAutoDestroy(false);

	if (TickWhenPaused)
	{
		newComponent->PrimaryComponentTick.bTickEvenWhenPaused = true;
		newComponent->SetForceSolo(true);
	}

	return newComponent;
}
PRAGMA_ENABLE_OPTIMIZATION