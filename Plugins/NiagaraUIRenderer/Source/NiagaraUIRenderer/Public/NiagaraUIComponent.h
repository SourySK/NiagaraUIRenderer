// Copyright 2023 - Michal Smoleň

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "NiagaraWidgetProperties.h"

#include "NiagaraUIComponent.generated.h"

class SNiagaraUISystemWidget;
class FNiagaraEmitterInstance;

/**
 * 
 */
UCLASS()
class NIAGARAUIRENDERER_API UNiagaraUIComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:
	void SetAutoActivateParticle(bool AutoActivate);

	void RequestActivateSystem(bool Reset);
	void RequestDeactivateSystem();
	
	void SetTransformationForUIRendering(FVector2D Location, FVector2f Scale, float Angle);

	void RenderUI(SNiagaraUISystemWidget* NiagaraWidget, float ScaleFactor, FVector2f ParentTopLeft, const FNiagaraWidgetProperties* WidgetProperties);

	void AddSpriteRendererData(SNiagaraUISystemWidget* NiagaraWidget, TSharedRef<const FNiagaraEmitterInstance> EmitterInst,
								class UNiagaraSpriteRendererProperties* SpriteRenderer, float ScaleFactor, FVector2f ParentTopLeft, const FNiagaraWidgetProperties* WidgetProperties);

	void AddRibbonRendererData(SNiagaraUISystemWidget* NiagaraWidget, TSharedRef<const FNiagaraEmitterInstance> EmitterInst,
                                class UNiagaraRibbonRendererProperties* RibbonRenderer, float ScaleFactor, FVector2f ParentTopLeft, const FNiagaraWidgetProperties* WidgetProperties);

	
	
private:
	bool AutoActivateParticle = false;

	// Indicates if the position of the transform was ever set based on the UI position
	bool HasSetTransform = false;
	
};
