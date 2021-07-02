// Copyright 2021 - Michal Smoleň

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "NiagaraWidgetProperties.h"

#include "NiagaraUIComponent.generated.h"

class SNiagaraUISystemWidget;

/**
 * 
 */
UCLASS()
class NIAGARAUIRENDERER_API UNiagaraUIComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:
	void SetTransformationForUIRendering(FVector2D Location, FVector2D Scale, float Angle);

	void RenderUI(SNiagaraUISystemWidget* NiagaraWidget, float ScaleFactor, FVector2D ParentTopLeft, const FNiagaraWidgetProperties* WidgetProperties);

	void AddSpriteRendererData(SNiagaraUISystemWidget* NiagaraWidget, TSharedRef<const FNiagaraEmitterInstance, ESPMode::ThreadSafe> EmitterInst,
								class UNiagaraSpriteRendererProperties* SpriteRenderer, float ScaleFactor, FVector2D ParentTopLeft, const FNiagaraWidgetProperties* WidgetProperties);

	void AddRibbonRendererData(SNiagaraUISystemWidget* NiagaraWidget, TSharedRef<const FNiagaraEmitterInstance, ESPMode::ThreadSafe> EmitterInst,
                                class UNiagaraRibbonRendererProperties* RibbonRenderer, float ScaleFactor, FVector2D ParentTopLeft, const FNiagaraWidgetProperties* WidgetProperties);

	
	
private:
	bool ShouldActivateParticle = false;
	
};
