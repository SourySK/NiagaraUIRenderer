// Copyright 2024 - Michal Smoleň

#include "SNiagaraUISystemWidget.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraWidgetProperties.h"
#include "NiagaraUIComponent.h"

TMap<TObjectPtr<UMaterialInterface>, TSharedPtr<FSlateMaterialBrush>> SNiagaraUISystemWidget::MaterialBrushMap;

void SNiagaraUISystemWidget::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
    SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "DesiredSize", DesiredSizeAttribute, EInvalidateWidgetReason::Layout);
    SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "ColorAndOpacity", ColorAndOpacityAttribute, EInvalidateWidgetReason::Paint);
}

void SNiagaraUISystemWidget::Construct(const FArguments& Args)
{
    DesiredSizeAttribute.Assign(*this, Args._DesiredSize);
    ColorAndOpacityAttribute.Assign(*this, Args._ColorAndOpacity);
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

    float A, B, C, D;
    SlateRenderTransform.GetMatrix().GetMatrix(A, B, C, D);
    const float AdditionalAngle = D < 0.f ? 180.f : 0.f;
    const float Angle = FMath::RadiansToDegrees(FMath::Atan(C / D)) + AdditionalAngle;

    FNiagaraUIRenderProperties RenderProperties = FNiagaraUIRenderProperties(LayoutScale, ParentTopLeft, InWidgetStyle.GetColorAndOpacityTint() * ColorAndOpacityAttribute.Get().GetColor(InWidgetStyle));

    UNiagaraUIComponent* NiagaraUIComponent = NiagaraComponent.Get();

    NiagaraUIComponent->SetTransformationForUIRendering(Location2D, Scale2D.GetVector() / LayoutScale, Angle);
    NiagaraUIComponent->RenderUI(const_cast<SNiagaraUISystemWidget*>(this), RenderProperties, &WidgetProperties);

    return SMeshWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

FVector2D SNiagaraUISystemWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
    return DesiredSizeAttribute.Get();
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
    const TObjectPtr<UMaterialInterface>* FoundMaterial = WidgetProperties.MaterialRemapList->Find(Material);
    const bool FoundMaterialValid = FoundMaterial && *FoundMaterial != nullptr;
    UMaterialInterface* MaterialToUse = FoundMaterialValid ? FoundMaterial->Get() : Material;
    
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

void SNiagaraUISystemWidget::SetNiagaraComponentReference(TWeakObjectPtr<UNiagaraUIComponent> NiagaraComponentIn)
{
    if (!ensure(NiagaraComponentIn != nullptr))
        return;

    NiagaraComponent = NiagaraComponentIn;
}

void SNiagaraUISystemWidget::SetNiagaraWidgetProperties(FNiagaraWidgetProperties Properties)
{
    WidgetProperties = Properties;
}

void SNiagaraUISystemWidget::SetDesiredSize(FVector2D NewDesiredSize)
{
    DesiredSizeAttribute.Set(*this, NewDesiredSize);
}

void SNiagaraUISystemWidget::SetColorAndOpacity(FLinearColor NewColorAndOpacity)
{
    ColorAndOpacityAttribute.Set(*this, NewColorAndOpacity);
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
