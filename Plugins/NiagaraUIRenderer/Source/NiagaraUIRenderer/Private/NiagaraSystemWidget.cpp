// Copyright 2022 - Michal Smoleň

#include "NiagaraSystemWidget.h"
#include "SNiagaraUISystemWidget.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraUIActor.h"
#include "NiagaraUIComponent.h"

UNiagaraSystemWidget::UNiagaraSystemWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bIsVolatile = true;
}

TSharedRef<SWidget> UNiagaraSystemWidget::RebuildWidget()
{
	NiagaraSlateWidget = SNew(SNiagaraUISystemWidget);

	InitializeNiagaraUI();

	return NiagaraSlateWidget.ToSharedRef();
}

void UNiagaraSystemWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (!NiagaraSlateWidget.IsValid())
	{
		return;
	}

	if (!NiagaraActor || !NiagaraComponent)
		InitializeNiagaraUI();
	
}

void UNiagaraSystemWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	
	NiagaraSlateWidget.Reset();

	if (NiagaraActor)
		NiagaraActor->Destroy();
}

#if WITH_EDITOR
const FText UNiagaraSystemWidget::GetPaletteCategory()
{
	return NSLOCTEXT("NiagaraUIRenderer", "Palette Category", "Niagara");
}

void UNiagaraSystemWidget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.MemberProperty)
	{
		const FName PropertyName = PropertyChangedEvent.MemberProperty->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UNiagaraSystemWidget, NiagaraSystemReference)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UNiagaraSystemWidget, MaterialRemapList)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UNiagaraSystemWidget, AutoActivate)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UNiagaraSystemWidget, FakeDepthScale)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UNiagaraSystemWidget, FakeDepthScaleDistance)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UNiagaraSystemWidget, PassDynamicParametersFromRibbon))
		{
			InitializeNiagaraUI();
		}
	}
}
#endif

void UNiagaraSystemWidget::InitializeNiagaraUI()
{
	if (UWorld* World = GetWorld())
	{
		if (!World->PersistentLevel)
			return;

			
		if (!NiagaraActor)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.ObjectFlags |= RF_Transient;
			
			NiagaraActor = World->SpawnActor<ANiagaraUIActor>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		}

		NiagaraComponent = NiagaraActor->SpawnNewNiagaraUIComponent(NiagaraSystemReference, AutoActivate, ShowDebugSystemInWorld, TickWhenPaused);

		NiagaraSlateWidget->SetNiagaraComponentReference(NiagaraComponent, FNiagaraWidgetProperties(&MaterialRemapList, AutoActivate, ShowDebugSystemInWorld, PassDynamicParametersFromRibbon, FakeDepthScale, FakeDepthScaleDistance));
	}
}

void UNiagaraSystemWidget::ActivateSystem(bool Reset)
{
	if (NiagaraComponent)
		NiagaraComponent->Activate(Reset);
}

void UNiagaraSystemWidget::DeactivateSystem()
{
	if (NiagaraComponent)
		NiagaraComponent->Deactivate();
}

UNiagaraUIComponent* UNiagaraSystemWidget::GetNiagaraComponent()
{
	return NiagaraComponent;
}

void UNiagaraSystemWidget::UpdateNiagaraSystemReference(UNiagaraSystem* NewNiagaraSystem)
{
	NiagaraSystemReference = NewNiagaraSystem;

	if (NiagaraComponent)
	{
		NiagaraComponent->SetAsset(NewNiagaraSystem);
		NiagaraComponent->ResetSystem();
	}
}

void UNiagaraSystemWidget::UpdateTickWhenPaused(bool NewTickWhenPaused)
{
	TickWhenPaused = NewTickWhenPaused;

	if (NiagaraComponent)
	{
		NiagaraComponent->SetTickableWhenPaused(NewTickWhenPaused);
		NiagaraComponent->SetForceSolo(NewTickWhenPaused);
		NiagaraComponent->ResetSystem();
	}
}
