// Copyright 2023 - Michal Smoleň

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "NiagaraWidgetProperties.h"

#include "NiagaraUIComponent.generated.h"

class SNiagaraUISystemWidget;
class FNiagaraEmitterInstance;

struct FNiagaraUIRenderProperties
{
public:
	FNiagaraUIRenderProperties(float InScaleFactor, FVector2f InParentTopLeft, FLinearColor InTint)
		: ScaleFactor(InScaleFactor), ParentTopLeft(InParentTopLeft), Tint(InTint)
		{ }
	
public:
	float ScaleFactor;
	FVector2f ParentTopLeft;
	FLinearColor Tint;
};

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

	void RenderUI(SNiagaraUISystemWidget* NiagaraWidget, const FNiagaraUIRenderProperties& RenderProperties, const FNiagaraWidgetProperties* WidgetProperties);

	void AddSpriteRendererData(SNiagaraUISystemWidget* NiagaraWidget, TSharedRef<const FNiagaraEmitterInstance> EmitterInst,
								class UNiagaraSpriteRendererProperties* SpriteRenderer, const FNiagaraUIRenderProperties& RenderProperties, const FNiagaraWidgetProperties* WidgetProperties);

	void AddRibbonRendererData(SNiagaraUISystemWidget* NiagaraWidget, TSharedRef<const FNiagaraEmitterInstance> EmitterInst,
                                class UNiagaraRibbonRendererProperties* RibbonRenderer, const FNiagaraUIRenderProperties& RenderProperties, const FNiagaraWidgetProperties* WidgetProperties);

	
	
private:
	bool AutoActivateParticle = false;

	// Indicates if the position of the transform was ever set based on the UI position
	bool HasSetTransform = false;

	// Cached angle of the widget
	float WidgetRotationAngle;
	
};
