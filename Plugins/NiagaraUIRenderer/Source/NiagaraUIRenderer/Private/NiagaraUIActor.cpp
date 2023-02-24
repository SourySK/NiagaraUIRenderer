// Copyright 2023 - Michal Smoleň

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
	
	// Don't use unreal activation, we'll do it ourself
	NewComponent->SetAutoActivate(false);

	NewComponent->SetupAttachment(RootComponent);
	NewComponent->SetHiddenInGame(!ShowDebugSystem);
	NewComponent->RegisterComponent();
	NewComponent->SetAutoActivateParticle(AutoActivate);	
	NewComponent->SetAsset(NiagaraSystemTemplate);	
	NewComponent->SetAutoDestroy(false);

	if (TickWhenPaused)
	{
		NewComponent->PrimaryComponentTick.bTickEvenWhenPaused = true;
		NewComponent->SetForceSolo(true);
	}

	return NewComponent;
}