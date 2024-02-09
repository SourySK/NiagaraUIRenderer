// Copyright 2023 - Michal Smoleň

#include "NiagaraSystemWidget.h"

#include "NiagaraSystem.h"
#include "SNiagaraUISystemWidget.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraUIActor.h"
#include "NiagaraUIComponent.h"
#include "Engine/World.h"

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

	NiagaraActor = nullptr;
	NiagaraComponent = nullptr;
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
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UNiagaraSystemWidget, PassDynamicParametersFromRibbon)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UNiagaraSystemWidget, DesiredWidgetSize))
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
		NiagaraSlateWidget->SetDesiredSize(DesiredWidgetSize);
	}
}

void UNiagaraSystemWidget::ActivateSystem(bool Reset)
{
	if (NiagaraComponent)
		NiagaraComponent->RequestActivateSystem(Reset);
}

void UNiagaraSystemWidget::DeactivateSystem()
{
	if (NiagaraComponent)
		NiagaraComponent->RequestDeactivateSystem();
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

void UNiagaraSystemWidget::SetDesiredWidgetSize(FVector2D NewDesiredSize)
{
	DesiredWidgetSize = NewDesiredSize;

	if (NiagaraSlateWidget.IsValid())
	{
		NiagaraSlateWidget->SetDesiredSize(DesiredWidgetSize);
	}
}

void UNiagaraSystemWidget::SetRemapMaterial(UMaterialInterface* OriginalMaterial, UMaterialInterface* RemapMaterial)
{
	if (OriginalMaterial && RemapMaterial)
	{
		MaterialRemapList.Emplace(OriginalMaterial, RemapMaterial);
	}
}

UMaterialInterface* UNiagaraSystemWidget::GetRemapMaterial(UMaterialInterface* OriginalMaterial)
{
	if (UMaterialInterface** FoundMaterial = MaterialRemapList.Find(OriginalMaterial))
	{
		return *FoundMaterial;
	}

	return nullptr;
}
