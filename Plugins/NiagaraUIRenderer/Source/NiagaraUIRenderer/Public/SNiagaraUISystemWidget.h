// Copyright 2024 - Michal Smoleň

#pragma once

#include "NiagaraWidgetProperties.h"
#include "SlateMaterialBrush.h"
#include "Slate/SMeshWidget.h"

class UNiagaraUIComponent;
class UMaterialInterface;

/**
 * 
 */
class NIAGARAUIRENDERER_API SNiagaraUISystemWidget : public SMeshWidget
{	
	SLATE_DECLARE_WIDGET(SNiagaraUISystemWidget, SMeshWidget)
	
public:
	SLATE_BEGIN_ARGS(SNiagaraUISystemWidget)
		: _DesiredSize(FVector2D(256., 256.))
		, _ColorAndOpacity(FLinearColor::White)
		{ }
		
		SLATE_ATTRIBUTE(FVector2D, DesiredSize)
		SLATE_ATTRIBUTE(FSlateColor, ColorAndOpacity)
		
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);

	virtual ~SNiagaraUISystemWidget() override;

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

	void AddRenderData(FSlateVertex** OutVertexData, SlateIndex** OutIndexData, UMaterialInterface* Material, int32 NumVertexData, int32 NumIndexData);
	
	void ClearRenderData();

	TSharedPtr<FSlateMaterialBrush> CreateSlateMaterialBrush(UMaterialInterface* Material);

	void CheckForInvalidBrushes();

	void SetNiagaraComponentReference(TWeakObjectPtr<UNiagaraUIComponent> NiagaraComponentIn);
	void SetNiagaraWidgetProperties(FNiagaraWidgetProperties Properties);

	void SetDesiredSize(FVector2D NewDesiredSize);
	void SetColorAndOpacity(FLinearColor NewColorAndOpacity);

	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
	//~ End FGCObject Interface

private:
	TWeakObjectPtr<UNiagaraUIComponent> NiagaraComponent;

	static TMap<TObjectPtr<UMaterialInterface>, TSharedPtr<FSlateMaterialBrush>> MaterialBrushMap;

	FNiagaraWidgetProperties WidgetProperties = FNiagaraWidgetProperties(nullptr, true, false, false, false, 1.f);

	TSlateAttribute<FVector2D> DesiredSizeAttribute = TSlateAttribute<FVector2D>(*this);
	
	TSlateAttribute<FSlateColor> ColorAndOpacityAttribute = TSlateAttribute<FSlateColor>(*this);
};
