// Copyright 2023 - Michal Smoleň

#include "SNiagaraUISystemWidget.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraWidgetProperties.h"
#include "NiagaraUIComponent.h"

TMap<TObjectPtr<UMaterialInterface>, TSharedPtr<FSlateMaterialBrush>> SNiagaraUISystemWidget::MaterialBrushMap;

void SNiagaraUISystemWidget::Construct(const FArguments& Args)
{
}

SNiagaraUISystemWidget::~SNiagaraUISystemWidget()
{
    ClearRenderData();
    CheckForInvalidBrushes();
}

int32 SNiagaraUISystemWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(SNiagaraUISystemWidget::OnPaint);

    if (!NiagaraComponent.IsValid())
        return LayerId;

    const FSlateLayoutTransform LayoutTransform = AllottedGeometry.GetAccumulatedLayoutTransform();
    
    const float LayoutScale = LayoutTransform.GetScale();

    const FVector2f ParentTopLeft = FVector2f(MyCullingRect.Left, MyCullingRect.Top);

    const FSlateRenderTransform SlateRenderTransform = AllottedGeometry.GetAccumulatedRenderTransform();
    
    const FVector2D Location2D = (AllottedGeometry.GetAbsolutePositionAtCoordinates(FVector2D(0.5f, 0.5f)) - FVector2D(ParentTopLeft)) / LayoutScale;
    const FScale2D Scale2D = SlateRenderTransform.GetMatrix().GetScale();
    const float Angle = SlateRenderTransform.GetMatrix().GetRotationAngle();

    UNiagaraUIComponent* NiagaraUIComponent = NiagaraComponent.Get();

    NiagaraUIComponent->SetTransformationForUIRendering(Location2D, Scale2D.GetVector() / LayoutScale, Angle);
    NiagaraUIComponent->RenderUI(const_cast<SNiagaraUISystemWidget*>(this), LayoutScale, ParentTopLeft, &WidgetProperties);

    return SMeshWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

FVector2D SNiagaraUISystemWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
    return DesiredSize;
}

void SNiagaraUISystemWidget::AddRenderData(FSlateVertex** OutVertexData, SlateIndex** OutIndexData, UMaterialInterface* Material, int32 NumVertexData, int32 NumIndexData)
{
    if (NumVertexData < 1 || NumIndexData < 1)
        return;
    
    FRenderData& NewRenderData = RenderData[RenderData.Add(FRenderData())];

    NewRenderData.VertexData.AddUninitialized(NumVertexData);
    *OutVertexData = &NewRenderData.VertexData[0];
    
    NewRenderData.IndexData.AddUninitialized(NumIndexData);
    *OutIndexData = &NewRenderData.IndexData[0];

    if (Material)
    {
        NewRenderData.Brush = CreateSlateMaterialBrush(Material);
        NewRenderData.RenderingResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*NewRenderData.Brush);
    }
}


void SNiagaraUISystemWidget::ClearRenderData()
{
    RenderData.Empty();
}

TSharedPtr<FSlateMaterialBrush> SNiagaraUISystemWidget::CreateSlateMaterialBrush(UMaterialInterface* Material)
{
    UMaterialInterface* MaterialToUse = WidgetProperties.MaterialRemapList->Contains(Material) && WidgetProperties.MaterialRemapList->Find(Material) != nullptr ? *WidgetProperties.MaterialRemapList->Find(Material) : Material;
    
    if (MaterialBrushMap.Contains(MaterialToUse))
    {
        const auto MapElement = MaterialBrushMap.Find(MaterialToUse);

        if (MapElement->IsValid())
        {
            return *MapElement;
        }
    }
    
    TSharedPtr<FSlateMaterialBrush> NewElement = MakeShareable(new FSlateMaterialBrush(*MaterialToUse, FVector2D(1.f, 1.f)));

    MaterialBrushMap.Add(MaterialToUse, NewElement);
    return NewElement;
}

void SNiagaraUISystemWidget::CheckForInvalidBrushes()
{
    TArray<UMaterialInterface*> RemoveMaterials;
    for (const auto& Brush : MaterialBrushMap)
    {
        if (Brush.Value.GetSharedReferenceCount() <= 1)
        {
            RemoveMaterials.Add(Brush.Key);
        }
    }

    for (const auto MaterialToRemove : RemoveMaterials)
    {
        MaterialBrushMap.Remove(MaterialToRemove);
    }
}

void SNiagaraUISystemWidget::SetNiagaraComponentReference(TWeakObjectPtr<UNiagaraUIComponent> NiagaraComponentIn, FNiagaraWidgetProperties Properties)
{
    if (!ensure(NiagaraComponentIn != nullptr))
        return;

    WidgetProperties = Properties;

    NiagaraComponent = NiagaraComponentIn;
}

void SNiagaraUISystemWidget::SetDesiredSize(FVector2D NewDesiredSize)
{
    DesiredSize = NewDesiredSize;
}

void SNiagaraUISystemWidget::AddReferencedObjects(FReferenceCollector& Collector)
{
    SMeshWidget::AddReferencedObjects(Collector);

    for (TTuple<TObjectPtr<UMaterialInterface>, TSharedPtr<FSlateMaterialBrush>>& element : MaterialBrushMap)
    {
        Collector.AddReferencedObject(element.Key);

        if (element.Value.IsValid())
        {
            element.Value->AddReferencedObjects(Collector);
        }
    }
}

FString SNiagaraUISystemWidget::GetReferencerName() const
{
    return TEXT("SNiagaraUISystemWidget");
}
