// Copyright 2023 - Michal Smoleň

#pragma once

#include "CoreMinimal.h"
#include "NiagaraWidgetProperties.h"
#include "Runtime/UMG/Public/Components/Widget.h"
#include "NiagaraSystemWidget.generated.h"

class SNiagaraUISystemWidget;
class UMaterialInterface;

/**
 The Niagara System Widget allows to render niagara particle system directly into the UI. Only sprite and ribbon CPU particles are supported.
 */
UCLASS()
class NIAGARAUIRENDERER_API UNiagaraSystemWidget : public UWidget
{
	GENERATED_BODY()

public:
	UNiagaraSystemWidget(const FObjectInitializer& ObjectInitializer);
	
	virtual TSharedRef<SWidget> RebuildWidget() override;
	
	virtual void SynchronizeProperties() override;
	
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void InitializeNiagaraUI();

public:
	// Activate Niagara System with option to reset the simulation
	UFUNCTION(BlueprintCallable, Category = "Niagara UI Renderer")
	void ActivateSystem(bool Reset);

	// Deactivate Niagara System
	UFUNCTION(BlueprintCallable, Category = "Niagara UI Renderer")
    void DeactivateSystem();

	// Return Niagara Component reference for the particle system.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Niagara UI Renderer")
    class UNiagaraUIComponent* GetNiagaraComponent();

	// Updates the Niagara System reference. This will cause the particle system to reset
	UFUNCTION(BlueprintCallable, Category = "Niagara UI Renderer")
	void UpdateNiagaraSystemReference(class UNiagaraSystem* NewNiagaraSystem);

	// Updates Tick When Paused - Should be this particle system updated even when the game is paused? Note that this will reset the particle simulation
	UFUNCTION(BlueprintCallable, Category = "Niagara UI Renderer")
	void UpdateTickWhenPaused(bool NewTickWhenPaused);

	// Updates the desired widget size. If calling from code call this instead of setting the DesiredWidgetSize directly.
	UFUNCTION(BlueprintCallable, Category = "Niagara UI Renderer")
	void SetDesiredWidgetSize(FVector2D NewDesiredSize);
	
	/** 
	 *	Sets / Updates a remap material for a source particle material
	 *	@param	OriginalMaterial	Source material specified in the Niagara Emitter that should be remapped to a new one
	 *	@param	RemapMaterial		A new remap material that should be used when rendering UI
	 */
	UFUNCTION(BlueprintCallable, Category = "Niagara UI Renderer")
	void SetRemapMaterial(UMaterialInterface* OriginalMaterial, UMaterialInterface* RemapMaterial);
	
	// Returns a remap material for a given original particle material specified in the Niagara Emitter 
	UFUNCTION(BlueprintCallable, Category = "Niagara UI Renderer")
	UMaterialInterface* GetRemapMaterial(UMaterialInterface* OriginalMaterial);

public:
	// Reference to the niagara system asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara UI Renderer", DisplayName = "Niagara System", BlueprintSetter = UpdateNiagaraSystemReference)
	class UNiagaraSystem* NiagaraSystemReference;

	/*
		List of material references used to remap materials on the particle system, to materials with "User Interface" material domain.

		Every Key (Material on the left) will be remapped to it's Value (Material on the right)

		This is useful for keeping the particle system rendering correctly in the niagara editor and in the world, while it still can be used as UI particle system.

		The alternative is to apply materials with "User Interface" material domain directly in niagara renderers, but this will result in particle system
		not rendering correctly, if used outside UI renderer.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara UI Renderer")
	TMap<TObjectPtr<UMaterialInterface>, UMaterialInterface*> MaterialRemapList;

	// Should be this particle system automatically activated?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara UI Renderer")
	bool AutoActivate = true;

	// Should be this particle system updated even when the game is paused?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara UI Renderer", BlueprintSetter = UpdateTickWhenPaused)
	bool TickWhenPaused = false;

	// The size of this particle widget used when calculating it's desired size. Don't set directly from code, call SetDesiredWidgetSize instead.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara UI Renderer", meta = (ClampMin = 0.f, UIMax = 4096.f))
	FVector2D DesiredWidgetSize = FVector2D(256., 256.);

	// Scale particles based on their position in Y-axis (towards the camera)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara UI Renderer", AdvancedDisplay)
	bool FakeDepthScale = false;

	// Fake distance from camera if the particle is at 0 0 0 - Particles will be getting bigger quicker the lower this number is
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara UI Renderer", AdvancedDisplay, meta = (EditCondition = "FakeDepthScale"))
	float FakeDepthScaleDistance = 1000.f;

	// Show debug particle system we're rendering in the game world. It'll be near 0 0 0
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara UI Renderer", AdvancedDisplay)
	bool ShowDebugSystemInWorld = false;

	// Should the dynamic parameters R and G channels be passed as UV1 coordinates from ribbon renderers? This replaces the generated UV1 coordinates, which on the use case might be worth the tradeoff.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara UI Renderer", AdvancedDisplay)
	bool PassDynamicParametersFromRibbon = false;

	// Disable warnings for this Widget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Niagara UI Renderer", AdvancedDisplay)
	bool DisableWarnings = false;

private:
	TSharedPtr<SNiagaraUISystemWidget> NiagaraSlateWidget;

	UPROPERTY()
	class ANiagaraUIActor* NiagaraActor;

	UPROPERTY()
	class UNiagaraUIComponent* NiagaraComponent;
};
