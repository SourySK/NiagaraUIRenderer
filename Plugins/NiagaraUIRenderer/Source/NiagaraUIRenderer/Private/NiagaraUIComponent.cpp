// Copyright 2023 - Michal Smoleň

#include "NiagaraUIComponent.h"
#include "Stats/Stats.h"
#include "NiagaraRenderer.h"
#include "NiagaraRibbonRendererProperties.h"
#include "NiagaraSpriteRendererProperties.h"
#include "NiagaraSystemInstanceController.h"
#include "SNiagaraUISystemWidget.h"


DECLARE_STATS_GROUP(TEXT("NiagaraUI"), STATGROUP_NiagaraUI, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Generate Sprite Data"), STAT_GenerateSpriteData, STATGROUP_NiagaraUI);
DECLARE_CYCLE_STAT(TEXT("Generate Ribbon Data"), STAT_GenerateRibbonData, STATGROUP_NiagaraUI);

//PRAGMA_DISABLE_OPTIMIZATION

void UNiagaraUIComponent::SetAutoActivateParticle(bool AutoActivate)
{
	AutoActivateParticle = AutoActivate;
}

void UNiagaraUIComponent::RequestActivateSystem(bool Reset)
{
	// If we already have a valid transform activate us right now
	if (HasSetTransform)
	{
		Activate(Reset);
		return;
	}

	// If not, wait until we get one
	SetAutoActivateParticle(true);
}

void UNiagaraUIComponent::RequestDeactivateSystem()
{
	Deactivate();
	SetAutoActivateParticle(false);
}

void UNiagaraUIComponent::SetTransformationForUIRendering(FVector2D Location, FVector2f Scale, float Angle)
{
	const FVector NewLocation(Location.X, 0.f, -Location.Y);
	const FVector NewScale(Scale.X, 1.f, Scale.Y);
	const FRotator NewRotation(FMath::RadiansToDegrees(Angle), 0.f, 0.f);;
	
	SetRelativeTransform(FTransform(NewRotation, NewLocation, NewScale));

	if (AutoActivateParticle)
	{
		if (!IsActive())
			ActivateSystem();

		AutoActivateParticle = false;
	}

	HasSetTransform = true;
}

#if ENGINE_MINOR_VERSION < 1
struct FNiagaraRendererEntry
{
	FNiagaraRendererEntry(UNiagaraRendererProperties* PropertiesIn, TSharedRef<const FNiagaraEmitterInstance> EmitterInstIn, UNiagaraEmitter* EmitterIn)
		: RendererProperties(PropertiesIn), EmitterInstance(EmitterInstIn), Emitter(EmitterIn) {}
	UNiagaraRendererProperties* RendererProperties;
	TSharedRef<const FNiagaraEmitterInstance> EmitterInstance;
	UNiagaraEmitter* Emitter;
};
#else
struct FNiagaraRendererEntry
{
	FNiagaraRendererEntry(UNiagaraRendererProperties* PropertiesIn, TSharedRef<const FNiagaraEmitterInstance> EmitterInstIn, FVersionedNiagaraEmitter EmitterIn)
		: RendererProperties(PropertiesIn), EmitterInstance(EmitterInstIn), Emitter(EmitterIn) {}
	UNiagaraRendererProperties* RendererProperties;
	TSharedRef<const FNiagaraEmitterInstance> EmitterInstance;
	FVersionedNiagaraEmitter Emitter;
};
#endif

void UNiagaraUIComponent::RenderUI(SNiagaraUISystemWidget* NiagaraWidget, float ScaleFactor, FVector2f ParentTopLeft, const FNiagaraWidgetProperties* WidgetProperties)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNiagaraUIComponent::RenderUI);

	NiagaraWidget->ClearRenderData();

	if (!IsActive())
		return;

	if (!GetSystemInstanceController())
		return;
	
	TArray<FNiagaraRendererEntry> Renderers;

	for(TSharedRef<const FNiagaraEmitterInstance> EmitterInst : GetSystemInstanceController()->GetSystemInstance_Unsafe()->GetEmitters())
	{
		if (EmitterInst->IsDisabled())
			continue;
			
#if ENGINE_MINOR_VERSION < 1
		if (UNiagaraEmitter* Emitter = EmitterInst->GetCachedEmitter())
		{
			TArray<UNiagaraRendererProperties*> Properties = Emitter->GetRenderers();

			for (UNiagaraRendererProperties* Property : Properties)
			{
				FNiagaraRendererEntry NewEntry(Property, EmitterInst, Emitter);
                Renderers.Add(NewEntry);
			}
		}
#else
		#if ENGINE_MINOR_VERSION < 4
			FVersionedNiagaraEmitter Emitter = EmitterInst->GetCachedEmitter();
		#else
			FVersionedNiagaraEmitter Emitter = EmitterInst->GetVersionedEmitter();
		#endif

		if (Emitter.Emitter)
		{
			TArray<UNiagaraRendererProperties*> Properties = Emitter.GetEmitterData()->GetRenderers();

			for (UNiagaraRendererProperties* Property : Properties)
			{
				FNiagaraRendererEntry NewEntry(Property, EmitterInst, Emitter);
				Renderers.Add(NewEntry);
			}
		}
#endif
	}

	Algo::Sort(Renderers, [] (FNiagaraRendererEntry& FirstElement, FNiagaraRendererEntry& SecondElement) {return FirstElement.RendererProperties->SortOrderHint < SecondElement.RendererProperties->SortOrderHint;});
			
	for (FNiagaraRendererEntry Renderer : Renderers)
	{
#if ENGINE_MINOR_VERSION < 1
		if (Renderer.RendererProperties && Renderer.RendererProperties->GetIsEnabled() && Renderer.RendererProperties->IsSimTargetSupported(Renderer.Emitter->SimTarget))
		{
			if (Renderer.Emitter->SimTarget == ENiagaraSimTarget::CPUSim)
			{
#else
		if (Renderer.RendererProperties && Renderer.RendererProperties->GetIsEnabled() && Renderer.RendererProperties->IsSimTargetSupported(Renderer.Emitter.GetEmitterData()->SimTarget))
		{
			if (Renderer.Emitter.GetEmitterData()->SimTarget == ENiagaraSimTarget::CPUSim)
			{
#endif
				
				if (UNiagaraSpriteRendererProperties* SpriteRenderer = Cast<UNiagaraSpriteRendererProperties>(Renderer.RendererProperties))
				{
					AddSpriteRendererData(NiagaraWidget, Renderer.EmitterInstance, SpriteRenderer, ScaleFactor, ParentTopLeft, WidgetProperties);
				}
				else if (UNiagaraRibbonRendererProperties* RibbonRenderer = Cast<UNiagaraRibbonRendererProperties>(Renderer.RendererProperties))
				{
					AddRibbonRendererData(NiagaraWidget, Renderer.EmitterInstance, RibbonRenderer, ScaleFactor, ParentTopLeft, WidgetProperties);                		
				}
			}
		}
	}
}

FORCEINLINE FVector2D FastRotate(const FVector2D Vector, float Sin, float Cos)
{
	return FVector2D(Cos * Vector.X - Sin * Vector.Y,
                     Sin * Vector.X + Cos * Vector.Y);
}


void UNiagaraUIComponent::AddSpriteRendererData(SNiagaraUISystemWidget* NiagaraWidget, TSharedRef<const FNiagaraEmitterInstance> EmitterInst, UNiagaraSpriteRendererProperties* SpriteRenderer, float ScaleFactor, FVector2f ParentTopLeft, const FNiagaraWidgetProperties* WidgetProperties)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNiagaraUIComponent::AddSpriteRendererData);
	SCOPE_CYCLE_COUNTER(STAT_GenerateSpriteData);
	FVector ComponentLocation = GetRelativeLocation();
	FVector ComponentScale = GetRelativeScale3D();
	FRotator ComponentRotation = GetRelativeRotation();
	float ComponentPitchRadians = FMath::DegreesToRadians(ComponentRotation.Pitch);

#if ENGINE_MINOR_VERSION < 4
	FNiagaraDataSet& DataSet = EmitterInst->GetData();
#else
	const FNiagaraDataSet& DataSet = EmitterInst->GetParticleData();
#endif
			
	
	if (!DataSet.IsCurrentDataValid())
		return;
	
	FNiagaraDataBuffer& ParticleData = DataSet.GetCurrentDataChecked();
	const int32 ParticleCount = ParticleData.GetNumInstances();

	if (ParticleCount < 1)
		return;

	
#if ENGINE_MINOR_VERSION < 1		
	bool LocalSpace = EmitterInst->GetCachedEmitter()->bLocalSpace;
#elif ENGINE_MINOR_VERSION < 4
	bool LocalSpace = EmitterInst->GetCachedEmitterData()->bLocalSpace;
#else
	bool LocalSpace = EmitterInst->GetVersionedEmitter().GetEmitterData()->bLocalSpace;
#endif
			

	const float FakeDepthScaler = 1 / WidgetProperties->FakeDepthScaleDistance;

	FVector2D SubImageSize = SpriteRenderer->SubImageSize;
	FVector2D SubImageDelta = FVector2D::UnitVector / SubImageSize;

	const auto PositionData = FNiagaraDataSetAccessor<FNiagaraPosition>::	CreateReader(DataSet, SpriteRenderer->PositionBinding.GetDataSetBindableVariable().GetName());
	const auto ColorData	= FNiagaraDataSetAccessor<FLinearColor>::		CreateReader(DataSet, SpriteRenderer->ColorBinding.GetDataSetBindableVariable().GetName());
	const auto VelocityData = FNiagaraDataSetAccessor<FVector3f>::			CreateReader(DataSet, SpriteRenderer->VelocityBinding.GetDataSetBindableVariable().GetName());
	const auto SizeData		= FNiagaraDataSetAccessor<FVector2f>::			CreateReader(DataSet, SpriteRenderer->SpriteSizeBinding.GetDataSetBindableVariable().GetName());
	const auto RotationData = FNiagaraDataSetAccessor<float>::				CreateReader(DataSet, SpriteRenderer->SpriteRotationBinding.GetDataSetBindableVariable().GetName());
	const auto SubImageData = FNiagaraDataSetAccessor<float>::				CreateReader(DataSet, SpriteRenderer->SubImageIndexBinding.GetDataSetBindableVariable().GetName());
	const auto DynamicMaterialData = FNiagaraDataSetAccessor<FVector4f>::	CreateReader(DataSet, SpriteRenderer->DynamicMaterialBinding.GetDataSetBindableVariable().GetName());

	auto GetParticlePosition2D = [&PositionData](int32 Index)
	{
		const FVector3f Position3D = PositionData.GetSafe(Index, FVector3f::ZeroVector);
		return FVector2f(Position3D.X, -Position3D.Z);
	};
	
	auto GetParticleDepth = [&PositionData](int32 Index)
	{
		return PositionData.GetSafe(Index, FVector::ZeroVector).Y;
	};	

	auto GetParticleColor = [&ColorData](int32 Index)
	{
		return ColorData.GetSafe(Index, FLinearColor::White);
	};
	
	auto GetParticleVelocity2D = [&VelocityData](int32 Index)
	{
		const FVector3f Velocity3D = VelocityData.GetSafe(Index, FVector3f::ZeroVector);
		return FVector2D(Velocity3D.X, Velocity3D.Z);
	};
	
	auto GetParticleSize = [&SizeData](int32 Index)
	{
		return SizeData.GetSafe(Index, FVector2f::UnitVector);
	};
	
	auto GetParticleRotation = [&RotationData](int32 Index)
	{
		return RotationData.GetSafe(Index, 0.f);
	};
	
	auto GetParticleSubImage = [&SubImageData](int32 Index)
	{
		return SubImageData.GetSafe(Index, 0.f);
	};
	
	auto GetDynamicMaterialData = [&DynamicMaterialData](int32 Index)
	{
		return DynamicMaterialData.GetSafe(Index, FVector4f(0.f, 0.f, 0.f, 0.f));
	};
	
	FSlateVertex* VertexData;	
	SlateIndex* IndexData;
	
	FSlateBrush Brush;
	UMaterialInterface* SpriteMaterial = SpriteRenderer->Material;

	NiagaraWidget->AddRenderData(&VertexData, &IndexData, SpriteMaterial, ParticleCount * 4, ParticleCount * 6);

	for (int ParticleIndex = 0; ParticleIndex < ParticleCount; ++ParticleIndex)
	{

		FVector2f ParticlePosition = GetParticlePosition2D(ParticleIndex) * ScaleFactor;
		FVector2f ParticleSize = GetParticleSize(ParticleIndex) * ScaleFactor;

		if (LocalSpace)
		{
			ParticlePosition *= FVector2f(ComponentScale.X, ComponentScale.Z);
			ParticlePosition  = ParticlePosition.GetRotated(-ComponentRotation.Pitch);
			ParticlePosition += ParentTopLeft;	
			ParticlePosition += FVector2f(ComponentLocation.X, -ComponentLocation.Z) * ScaleFactor;
			
			ParticleSize *= FVector2f(ComponentScale.X, ComponentScale.Z);
		}
		else
		{
			ParticlePosition += ParentTopLeft;					
		}


		if (WidgetProperties->FakeDepthScale)
		{
			const float ParticleDepth = (-GetParticleDepth(ParticleIndex) + WidgetProperties->FakeDepthScaleDistance) * FakeDepthScaler;
			ParticleSize *= ParticleDepth;
		}

		
		const FVector2f ParticleHalfSize = ParticleSize * 0.5;
		
		
		const FColor ParticleColor = GetParticleColor(ParticleIndex).ToFColor(true);
		
		
		float ParticleRotationSin, ParticleRotationCos;

		if (SpriteRenderer->Alignment == ENiagaraSpriteAlignment::VelocityAligned)
		{
			const FVector2D ParticleVelocity = GetParticleVelocity2D(ParticleIndex);

			ParticleRotationCos = FVector2D::DotProduct(ParticleVelocity.GetSafeNormal(), FVector2D(0.f, 1.f));
			const float SinSign = FMath::Sign(FVector2D::DotProduct(ParticleVelocity, FVector2D(1.f, 0.f)));
			
			if ( LocalSpace)
			{
				const float ParticleRotation = FMath::Acos(ParticleRotationCos * SinSign) - ComponentPitchRadians;
				FMath::SinCos(&ParticleRotationSin, &ParticleRotationCos, ParticleRotation);				
			}
			else
			{
				ParticleRotationSin = FMath::Sqrt(1 - ParticleRotationCos * ParticleRotationCos) * SinSign;
			}
		}
		else
		{
			float ParticleRotation = GetParticleRotation(ParticleIndex);
			
			if (LocalSpace)
				ParticleRotation -= ComponentRotation.Pitch;
			
			FMath::SinCos(&ParticleRotationSin, &ParticleRotationCos, FMath::DegreesToRadians(ParticleRotation));
		}
		
		
		FVector2D TextureCoordinates[4];
		
		if (SubImageSize != FVector2D(1.f, 1.f))
		{
			const float ParticleSubImage = GetParticleSubImage(ParticleIndex);
			const int Row = (int)FMath::Floor(ParticleSubImage / SubImageSize.X) % (int)SubImageSize.Y;
			const int Column = (int)(ParticleSubImage) % (int)(SubImageSize.X);

			const float LeftUV = SubImageDelta.X * Column;
			const float Right = SubImageDelta.X * (Column + 1);
			const float TopUV = SubImageDelta.Y * Row;
			const float BottomUV = SubImageDelta.Y * (Row + 1);
			
			TextureCoordinates[0] = FVector2D(LeftUV, TopUV);
			TextureCoordinates[1] = FVector2D(Right, TopUV);
			TextureCoordinates[2] = FVector2D(LeftUV, BottomUV);
			TextureCoordinates[3] = FVector2D(Right, BottomUV);
		}
		else
		{
			TextureCoordinates[0] = FVector2D(0.f, 0.f);
			TextureCoordinates[1] = FVector2D(1.f, 0.f);
			TextureCoordinates[2] = FVector2D(0.f, 1.f);
			TextureCoordinates[3] = FVector2D(1.f, 1.f);
		}

		
		const FVector4f MaterialData = GetDynamicMaterialData(ParticleIndex);

		FVector2D PositionArray[4];
		PositionArray[0] = FastRotate(FVector2D(-ParticleHalfSize.X, -ParticleHalfSize.Y), ParticleRotationSin, ParticleRotationCos);
		PositionArray[1] = FastRotate(FVector2D(ParticleHalfSize.X, -ParticleHalfSize.Y), ParticleRotationSin, ParticleRotationCos);
		PositionArray[2] = - PositionArray[1];
		PositionArray[3] = - PositionArray[0];
		
		const int VertexIndex = ParticleIndex * 4;
		const int indexIndex = ParticleIndex * 6;		
		
		
		for (int i = 0; i < 4; ++i)
		{
			VertexData[VertexIndex + i].Position = FVector2f(PositionArray[i]) + ParticlePosition;
			VertexData[VertexIndex + i].Color = ParticleColor;
			VertexData[VertexIndex + i].TexCoords[0] = TextureCoordinates[i].X;
			VertexData[VertexIndex + i].TexCoords[1] = TextureCoordinates[i].Y;
			VertexData[VertexIndex + i].TexCoords[2] = MaterialData.X;
			VertexData[VertexIndex + i].TexCoords[3] = MaterialData.Y;
		}
		
		
		IndexData[indexIndex] = VertexIndex;
		IndexData[indexIndex + 1] = VertexIndex + 1;
		IndexData[indexIndex + 2] = VertexIndex + 2;
		
		IndexData[indexIndex + 3] = VertexIndex + 2;
		IndexData[indexIndex + 4] = VertexIndex + 1;
		IndexData[indexIndex + 5] = VertexIndex + 3;
	}
}

void UNiagaraUIComponent::AddRibbonRendererData(SNiagaraUISystemWidget* NiagaraWidget, TSharedRef<const FNiagaraEmitterInstance> EmitterInst, UNiagaraRibbonRendererProperties* RibbonRenderer, float ScaleFactor, FVector2f ParentTopLeft, const FNiagaraWidgetProperties* WidgetProperties)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNiagaraUIComponent::AddRibbonRendererData);
	SCOPE_CYCLE_COUNTER(STAT_GenerateRibbonData);
	
	FVector ComponentLocation = GetRelativeLocation();
	FVector ComponentScale = GetRelativeScale3D();
	FRotator ComponentRotation = GetRelativeRotation();

#if ENGINE_MINOR_VERSION < 4
	FNiagaraDataSet& DataSet = EmitterInst->GetData();
#else
	const FNiagaraDataSet& DataSet = EmitterInst->GetParticleData();
#endif
			
	
	if (!DataSet.IsCurrentDataValid())
		return;
	
	FNiagaraDataBuffer& ParticleData = DataSet.GetCurrentDataChecked();
	const int32 ParticleCount = ParticleData.GetNumInstances();

	if (ParticleCount < 2)
		return;
	
#if ENGINE_MINOR_VERSION < 3
	const auto SortKeyReader = RibbonRenderer->SortKeyDataSetAccessor.GetReader(DataSet);

	if (!ensureMsgf(SortKeyReader.IsValid(), TEXT("Invalid Sort Key Reader encrountered while rendering ribbon particles. This can happen if the particle is missing \"Particle State\" module.")))
	{
		return;
	}
			
	auto RibbonLinkOrderSort = [&SortKeyReader](TArray<int32>& Container)
	{
		Container.Sort([&SortKeyReader](const int32& A, const int32& B) {	return (SortKeyReader[A] < SortKeyReader[B]); });
	};
#else
	const auto RibbonLinkOrderFloatData = RibbonRenderer->RibbonLinkOrderFloatAccessor.GetReader(DataSet);
	const auto RibbonLinkOrderInt32Data = RibbonRenderer->RibbonLinkOrderInt32Accessor.GetReader(DataSet);
			
	if (!ensureMsgf(RibbonLinkOrderFloatData.IsValid() || RibbonLinkOrderInt32Data.IsValid(), TEXT("Invalid Sort Key Reader encrountered while rendering ribbon particles. This can happen if the particle is missing \"Particle State\" module.")))
	{
		return;
	}

	auto RibbonLinkOrderSort = [&RibbonLinkOrderFloatData, &RibbonLinkOrderInt32Data](TArray<int32>& Container)
	{
		if (RibbonLinkOrderFloatData.IsValid())
		{
			Container.Sort([&RibbonLinkOrderFloatData](const uint32& A, const uint32& B) { return RibbonLinkOrderFloatData[A] < RibbonLinkOrderFloatData[B]; });
		}
		else
		{
			Container.Sort([&RibbonLinkOrderInt32Data](const uint32& A, const uint32& B) { return RibbonLinkOrderInt32Data[A] > RibbonLinkOrderInt32Data[B]; });
		}
	};
#endif

	const auto PositionData		= RibbonRenderer->PositionDataSetAccessor.GetReader(DataSet);
	const auto ColorData		= FNiagaraDataSetAccessor<FLinearColor>::CreateReader(DataSet, RibbonRenderer->ColorBinding.GetDataSetBindableVariable().GetName());
	const auto RibbonWidthData	= RibbonRenderer->SizeDataSetAccessor.GetReader(DataSet);
	const auto DynamicMaterialData = FNiagaraDataSetAccessor<FVector4f>::CreateReader(DataSet, RibbonRenderer->DynamicMaterialBinding.GetDataSetBindableVariable().GetName());
	
	const auto RibbonFullIDData = RibbonRenderer->RibbonFullIDDataSetAccessor.GetReader(DataSet);

	auto GetParticlePosition2D = [&PositionData](int32 Index)
	{
		const FVector3f Position3D = PositionData.GetSafe(Index, FVector3f::ZeroVector);
		return FVector2f(Position3D.X, -Position3D.Z);
	};	

	auto GetParticleColor = [&ColorData](int32 Index)
	{
		return ColorData.GetSafe(Index, FLinearColor::White);
	};
	
	auto GetParticleWidth = [&RibbonWidthData](int32 Index)
	{
		return RibbonWidthData.GetSafe(Index, 1.f);
	};
	
	auto GetDynamicMaterialData = [&DynamicMaterialData](int32 Index)
	{
		return DynamicMaterialData.GetSafe(Index, FVector4f(0.f, 0.f, 0.f, 0.f));
	};

#if ENGINE_MINOR_VERSION < 1		
	bool LocalSpace = EmitterInst->GetCachedEmitter()->bLocalSpace;
#elif ENGINE_MINOR_VERSION < 4
	bool LocalSpace = EmitterInst->GetCachedEmitterData()->bLocalSpace;
#else
	bool LocalSpace = EmitterInst->GetVersionedEmitter().GetEmitterData()->bLocalSpace;
#endif
			
	const bool FullIDs = RibbonFullIDData.IsValid();
	const bool MultiRibbons = FullIDs;

	auto AddRibbonVerts = [&](TArray<int32>& RibbonIndices)
	{
		const int32 NumParticlesInRibbon = RibbonIndices.Num();
		if (NumParticlesInRibbon < 2)
			return;
		
		FSlateVertex* VertexData;	
		SlateIndex* IndexData;
	
		FSlateBrush Brush;
		UMaterialInterface* SpriteMaterial = RibbonRenderer->Material;
		
		NiagaraWidget->AddRenderData(&VertexData, &IndexData, SpriteMaterial, (NumParticlesInRibbon) * 2, (NumParticlesInRibbon - 1) * 6);

		int32 CurrentVertexIndex = 0;
		int32 CurrentIndexIndex = 0;
		
			
		const int32 StartDataIndex = RibbonIndices[0];

		float TotalDistance = 0.0f;

		FVector2f LastPosition = GetParticlePosition2D(StartDataIndex);
		FVector2f CurrentPosition = FVector2f::ZeroVector;
		float CurrentWidth = 0.f;
		FVector2f LastToCurrentVector = FVector2f::ZeroVector;
		float LastToCurrentSize = 0.f;
		float LastU0 = 0.f;
		float LastU1 = 0.f;
		
		FVector2f LastParticleUIPosition = LastPosition * ScaleFactor;
		
		if (LocalSpace)
		{
			LastParticleUIPosition *= FVector2f(ComponentScale.X, ComponentScale.Z);
			LastParticleUIPosition = LastParticleUIPosition.GetRotated(-ComponentRotation.Pitch);
			LastParticleUIPosition += ParentTopLeft;
			
			LastParticleUIPosition += FVector2f(ComponentLocation.X, -ComponentLocation.Z) * ScaleFactor;
		}
		else
		{
			LastParticleUIPosition += ParentTopLeft;
		}
		
		int32 CurrentIndex = 1;
		int32 CurrentDataIndex = RibbonIndices[CurrentIndex];
		
		CurrentPosition = GetParticlePosition2D(CurrentDataIndex);
		LastToCurrentVector = CurrentPosition - LastPosition;
		LastToCurrentSize = LastToCurrentVector.Size();
		
		// Normalize LastToCurrVec
		LastToCurrentVector *= 1.f / LastToCurrentSize;
		

		const FColor InitialColor = GetParticleColor(StartDataIndex).ToFColor(true);
		const float InitialWidth = GetParticleWidth(StartDataIndex) * ScaleFactor;
		
		FVector2f InitialPositionArray[2];
		InitialPositionArray[0] = LastToCurrentVector.GetRotated(90.f) * InitialWidth * 0.5f;
		InitialPositionArray[1] = -InitialPositionArray[0];
		
		for (int i = 0; i < 2; ++i)
		{
			VertexData[CurrentVertexIndex + i].Position = InitialPositionArray[i] +  LastParticleUIPosition;
			VertexData[CurrentVertexIndex + i].Color = InitialColor;
			VertexData[CurrentVertexIndex + i].TexCoords[0] = 0;
			VertexData[CurrentVertexIndex + i].TexCoords[1] = 1 - i;
		}

		CurrentVertexIndex += 2;

		int32 NextIndex = CurrentIndex + 1;
		
		while (NextIndex <= NumParticlesInRibbon)
		{
			const bool IsLastParticle = NextIndex == NumParticlesInRibbon;
			
			const int32 NextDataIndex = IsLastParticle ? -1 : RibbonIndices[NextIndex];
			const FVector2f NextPosition = IsLastParticle ? FVector2f::ZeroVector : GetParticlePosition2D(NextDataIndex);	
			FVector2f CurrentToNextVector = IsLastParticle ? LastToCurrentVector : NextPosition - CurrentPosition;
			
			const float CurrentToNextSize = CurrentToNextVector.Size();		
			CurrentWidth = GetParticleWidth(CurrentDataIndex) * ScaleFactor;
			FColor CurrentColor = GetParticleColor(CurrentDataIndex).ToFColor(true);

			// Normalize CurrToNextVec
			CurrentToNextVector *= 1.f / CurrentToNextSize;
			
			const FVector2f CurrentTangent = (LastToCurrentVector + CurrentToNextVector).GetSafeNormal();
			
			TotalDistance += LastToCurrentSize;

			FVector2f CurrentPositionArray[2];
			CurrentPositionArray[0] = CurrentTangent.GetRotated(90.f) * CurrentWidth * 0.5f;
			CurrentPositionArray[1] = -CurrentPositionArray[0];

			FVector2f CurrentParticleUIPosition = CurrentPosition * ScaleFactor;

			if (LocalSpace)
			{
				CurrentParticleUIPosition *= FVector2f(ComponentScale.X, ComponentScale.Z);
				CurrentParticleUIPosition = CurrentParticleUIPosition.GetRotated(-ComponentRotation.Pitch);
				CurrentParticleUIPosition += ParentTopLeft;
				CurrentParticleUIPosition += FVector2f(ComponentLocation.X, -ComponentLocation.Z) * ScaleFactor;
			}
			else
			{
				CurrentParticleUIPosition += ParentTopLeft;
			}

			float CurrentU0 = 0.f;
		
			if (RibbonRenderer->UV0Settings.DistributionMode == ENiagaraRibbonUVDistributionMode::TiledOverRibbonLength)
			{
				CurrentU0 = LastU0 + LastToCurrentSize / RibbonRenderer->UV0Settings.TilingLength;
			}
			else
			{
				CurrentU0 = (float)CurrentIndex / ((float)NumParticlesInRibbon - 1.f);
			}

			FVector2f TextureCoordinates0[2];
			TextureCoordinates0[0] = FVector2f(CurrentU0, 1.f);
			TextureCoordinates0[1] = FVector2f(CurrentU0, 0.f);
			
			
            FVector2f TextureCoordinates1[2];

			if (WidgetProperties->PassDynamicParametersFromRibbon)
			{
				const FVector4f MaterialData = GetDynamicMaterialData(CurrentIndex);
				
				TextureCoordinates1[0] = FVector2f(MaterialData.X, MaterialData.Y);
				TextureCoordinates1[1] = FVector2f(MaterialData.X, MaterialData.Y);
			}
			else
			{
				float CurrentU1 = 0.f;
				
				if (RibbonRenderer->UV1Settings.DistributionMode == ENiagaraRibbonUVDistributionMode::TiledOverRibbonLength)
				{
					CurrentU1 = LastU1 + LastToCurrentSize / RibbonRenderer->UV1Settings.TilingLength;
				}
				else
				{
					CurrentU1 = (float)CurrentIndex / ((float)NumParticlesInRibbon - 1.f);
				}
			
				TextureCoordinates1[0] = FVector2f(CurrentU1, 1.f);
				TextureCoordinates1[1] = FVector2f(CurrentU1, 0.f);
				
				LastU1 = CurrentU1;
			}
			
			for (int i = 0; i < 2; ++i)
			{
				VertexData[CurrentVertexIndex + i].Position = CurrentPositionArray[i] + CurrentParticleUIPosition;
				VertexData[CurrentVertexIndex + i].Color = CurrentColor;
				VertexData[CurrentVertexIndex + i].TexCoords[0] = TextureCoordinates0[i].X;
				VertexData[CurrentVertexIndex + i].TexCoords[1] = TextureCoordinates0[i].Y;
				VertexData[CurrentVertexIndex + i].TexCoords[2] = TextureCoordinates1[i].X;
				VertexData[CurrentVertexIndex + i].TexCoords[3] = TextureCoordinates1[i].Y;
			}
			
			IndexData[CurrentIndexIndex] = CurrentVertexIndex - 2;
			IndexData[CurrentIndexIndex + 1] = CurrentVertexIndex - 1;
			IndexData[CurrentIndexIndex + 2] = CurrentVertexIndex;
		
			IndexData[CurrentIndexIndex + 3] = CurrentVertexIndex - 1;
			IndexData[CurrentIndexIndex + 4] = CurrentVertexIndex;
			IndexData[CurrentIndexIndex + 5] = CurrentVertexIndex + 1;
			

			CurrentVertexIndex += 2;
			CurrentIndexIndex += 6;
			
			CurrentIndex = NextIndex;
			CurrentDataIndex = NextDataIndex;
			LastPosition = CurrentPosition;
			LastParticleUIPosition = CurrentParticleUIPosition;
			CurrentPosition = NextPosition;
			LastToCurrentVector = CurrentToNextVector;
			LastToCurrentSize = CurrentToNextSize;
			LastU0 = CurrentU0;

			++NextIndex;
		}
	};

	if (!MultiRibbons)
	{
		TArray<int32> SortedIndices;
		for (int32 i = 0; i < ParticleCount; ++i)
		{
			SortedIndices.Add(i);
		}

		RibbonLinkOrderSort(SortedIndices);

		AddRibbonVerts(SortedIndices);
	}
	else
	{
		if (FullIDs)
		{
			TMap<FNiagaraID, TArray<int32>> MultiRibbonSortedIndices;

			for (int32 i = 0; i < ParticleCount; ++i)
			{
				TArray<int32>& Indices = MultiRibbonSortedIndices.FindOrAdd(RibbonFullIDData[i]);
				Indices.Add(i);
			}

			// Sort the ribbons by ID so that the draw order stays consistent.
			MultiRibbonSortedIndices.KeySort(TLess<FNiagaraID>());

			for (TPair<FNiagaraID, TArray<int32>>& Pair : MultiRibbonSortedIndices)
			{
				TArray<int32>& SortedIndices = Pair.Value;
				RibbonLinkOrderSort(SortedIndices);
				AddRibbonVerts(SortedIndices);
			};
		}
	}

}

//PRAGMA_ENABLE_OPTIMIZATION
