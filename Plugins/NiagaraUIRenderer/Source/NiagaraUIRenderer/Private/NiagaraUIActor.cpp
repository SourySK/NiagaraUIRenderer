// Copyright 2021 - Michal Smoleň

#include "NiagaraUIActor.h"
#include "NiagaraUIComponent.h"
#include "Components/SceneComponent.h"

ANiagaraUIActor::ANiagaraUIActor()
{
	if (!RootComponent)
		RootComponent = CreateDefaultSubobject<USceneComponent>(USceneComponent::GetDefaultSceneRootVariableName());
}

UNiagaraUIComponent* ANiagaraUIActor::SpawnNewNiagaraUIComponent(UNiagaraSystem* NiagaraSystemTemplate, bool AutoActivate, bool ShowDebugSystem, bool TickWhenPaused)
{
	// Find old component
	TArray<UNiagaraUIComponent*> OldComponents;
	GetComponents<UNiagaraUIComponent>(OldComponents);

	// And destroy it
	for (UNiagaraUIComponent* Component : OldComponents)
		Component->DestroyComponent();
	
	
	UNiagaraUIComponent* NewComponent = NewObject<UNiagaraUIComponent>(this);

	NewComponent->SetupAttachment(RootComponent);
	NewComponent->SetAutoActivate(AutoActivate);
	NewComponent->SetHiddenInGame(!ShowDebugSystem);
	NewComponent->RegisterComponent();
	NewComponent->SetAsset(NiagaraSystemTemplate);
	NewComponent->SetAutoDestroy(false);

	if (TickWhenPaused)
	{
		NewComponent->PrimaryComponentTick.bTickEvenWhenPaused = true;
		NewComponent->SetForceSolo(true);
	}

	return NewComponent;
}