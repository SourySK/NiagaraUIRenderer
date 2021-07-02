// Copyright 2021 - Michal Smoleň

#include "NiagaraUIComponent.h"
#include "Stats/Stats.h"
#include "NiagaraRenderer.h"
#include "NiagaraRibbonRendererProperties.h"
#include "NiagaraSpriteRendererProperties.h"
#include "SNiagaraUISystemWidget.h"


DECLARE_STATS_GROUP(TEXT("NiagaraUI"), STATGROUP_NiagaraUI, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Generate Sprite Data"), STAT_GenerateSpriteData, STATGROUP_NiagaraUI);
DECLARE_CYCLE_STAT(TEXT("Generate Ribbon Data"), STAT_GenerateRibbonData, STATGROUP_NiagaraUI);

//PRAGMA_DISABLE_OPTIMIZATION

void UNiagaraUIComponent::SetTransformationForUIRendering(FVector2D Location, FVector2D Scale, float Angle)
{
	const FVector NewLocation(Location.X, 0.f, -Location.Y);
	const FVector NewScale(Scale.X, 1.f, Scale.Y);
	const FRotator NewRotation(FMath::RadiansToDegrees(Angle), 0.f, 0.f);;
	
	SetRelativeTransform(FTransform(NewRotation, NewLocation, NewScale));

	if (bAutoActivate)
	{
		ActivateSystem();
		bAutoActivate = false;
	}
}

struct FNiagaraRendererEntry
{
	FNiagaraRendererEntry(UNiagaraRendererProperties* PropertiesIn, TSharedRef<const FNiagaraEmitterInstance, ESPMode::ThreadSafe> EmitterInstIn, UNiagaraEmitter* EmitterIn)
		: RendererProperties(PropertiesIn), EmitterInstance(EmitterInstIn), Emitter(EmitterIn) {}
	UNiagaraRendererProperties* RendererProperties;
	TSharedRef<const FNiagaraEmitterInstance, ESPMode::ThreadSafe> EmitterInstance;
	UNiagaraEmitter* Emitter;
};

void UNiagaraUIComponent::RenderUI(SNiagaraUISystemWidget* NiagaraWidget, float ScaleFactor, FVector2D ParentTopLeft, const FNiagaraWidgetProperties* WidgetProperties)
{
	NiagaraWidget->ClearRenderData();

	if (!IsActive())
		return;

	if (!GetSystemInstance())
		return;
	
	TArray<FNiagaraRendererEntry> Renderers;

	for(TSharedRef<const FNiagaraEmitterInstance, ESPMode::ThreadSafe> EmitterInst : GetSystemInstance()->GetEmitters())
	{
		if (UNiagaraEmitter* Emitter = EmitterInst->GetCachedEmitter())
		{
			TArray<UNiagaraRendererProperties*> Properties = Emitter->GetRenderers();

			for (UNiagaraRendererProperties* Property : Properties)
			{
				FNiagaraRendererEntry NewEntry(Property, EmitterInst, Emitter);
                Renderers.Add(NewEntry);
			}
		}
	}

	Algo::Sort(Renderers, [] (FNiagaraRendererEntry& FirstElement, FNiagaraRendererEntry& SecondElement) {return FirstElement.RendererProperties->SortOrderHint < SecondElement.RendererProperties->SortOrderHint;});
			
	for (FNiagaraRendererEntry Renderer : Renderers)
	{
		if (Renderer.RendererProperties && Renderer.RendererProperties->GetIsEnabled() && Renderer.RendererProperties->IsSimTargetSupported(Renderer.Emitter->SimTarget))
		{
			if (Renderer.Emitter->SimTarget == ENiagaraSimTarget::CPUSim)
			{
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


void UNiagaraUIComponent::AddSpriteRendererData(SNiagaraUISystemWidget* NiagaraWidget, TSharedRef<const FNiagaraEmitterInstance, ESPMode::ThreadSafe> EmitterInst, UNiagaraSpriteRendererProperties* SpriteRenderer, float ScaleFactor, FVector2D ParentTopLeft, const FNiagaraWidgetProperties* WidgetProperties)
{
	SCOPE_CYCLE_COUNTER(STAT_GenerateSpriteData);
	FVector ComponentLocation = GetRelativeLocation();
	FVector ComponentScale = GetRelativeScale3D();
	FRotator ComponentRotation = GetRelativeRotation();
	float ComponentPitchRadians = FMath::DegreesToRadians(ComponentRotation.Pitch);

	FNiagaraDataSet& DataSet = EmitterInst->GetData();
	FNiagaraDataBuffer& ParticleData = DataSet.GetCurrentDataChecked();
	const int32 ParticleCount = ParticleData.GetNumInstances();

	if (ParticleCount < 1)
		return;

	bool LocalSpace = EmitterInst->GetCachedEmitter()->bLocalSpace;

	const float FakeDepthScaler = 1 / WidgetProperties->FakeDepthScaleDistance;

	FVector2D SubImageSize = SpriteRenderer->SubImageSize;
	FVector2D SubImageDelta = FVector2D::UnitVector / SubImageSize;

	const auto PositionData = FNiagaraDataSetAccessor<FVector>::		CreateReader(DataSet, SpriteRenderer->PositionBinding.GetDataSetBindableVariable().GetName());
	const auto ColorData	= FNiagaraDataSetAccessor<FLinearColor>::	CreateReader(DataSet, SpriteRenderer->ColorBinding.GetDataSetBindableVariable().GetName());
	const auto VelocityData = FNiagaraDataSetAccessor<FVector>::		CreateReader(DataSet, SpriteRenderer->VelocityBinding.GetDataSetBindableVariable().GetName());
	const auto SizeData		= FNiagaraDataSetAccessor<FVector2D>::		CreateReader(DataSet, SpriteRenderer->SpriteSizeBinding.GetDataSetBindableVariable().GetName());
	const auto RotationData = FNiagaraDataSetAccessor<float>::			CreateReader(DataSet, SpriteRenderer->SpriteRotationBinding.GetDataSetBindableVariable().GetName());
	const auto SubImageData = FNiagaraDataSetAccessor<float>::			CreateReader(DataSet, SpriteRenderer->SubImageIndexBinding.GetDataSetBindableVariable().GetName());
	const auto DynamicMaterialData = FNiagaraDataSetAccessor<FVector4>::CreateReader(DataSet, SpriteRenderer->DynamicMaterialBinding.GetDataSetBindableVariable().GetName());

	auto GetParticlePosition2D = [&PositionData](int32 Index)
	{
		const FVector Position3D = PositionData.GetSafe(Index, FVector::ZeroVector);
		return FVector2D(Position3D.X, -Position3D.Z);
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
		const FVector Velocity3D = VelocityData.GetSafe(Index, FVector::ZeroVector);
		return FVector2D(Velocity3D.X, Velocity3D.Z);
	};
	
	auto GetParticleSize = [&SizeData](int32 Index)
	{
		return SizeData.GetSafe(Index, FVector2D::ZeroVector);
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
		return DynamicMaterialData.GetSafe(Index, FVector4(0.f, 0.f, 0.f, 0.f));
	};
	
	FSlateVertex* VertexData;	
	SlateIndex* IndexData;
	
	FSlateBrush Brush;
	UMaterialInterface* SpriteMaterial = SpriteRenderer->Material;

	NiagaraWidget->AddRenderData(&VertexData, &IndexData, SpriteMaterial, ParticleCount * 4, ParticleCount * 6);

	for (int ParticleIndex = 0; ParticleIndex < ParticleCount; ++ParticleIndex)
	{

		FVector2D ParticlePosition = GetParticlePosition2D(ParticleIndex) * ScaleFactor;
		FVector2D ParticleSize = GetParticleSize(ParticleIndex) * ScaleFactor;

		if (LocalSpace)
		{
			ParticlePosition *= FVector2D(ComponentScale.X, ComponentScale.Z);
			ParticlePosition  = ParticlePosition.GetRotated(-ComponentRotation.Pitch);
			ParticlePosition += ParentTopLeft;	
			ParticlePosition += FVector2D(ComponentLocation.X, -ComponentLocation.Z) * ScaleFactor;
			
			ParticleSize *= FVector2D(ComponentScale.X, ComponentScale.Z);
		}
		else
		{
			ParticlePosition +=  ParentTopLeft;					
		}


		if (WidgetProperties->FakeDepthScale)
		{
			const float ParticleDepth = (-GetParticleDepth(ParticleIndex) + WidgetProperties->FakeDepthScaleDistance) * FakeDepthScaler;
			ParticleSize *= ParticleDepth;
		}

		
		const FVector2D ParticleHalfSize = ParticleSize * 0.5;
		
		
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

		
		const FVector4 MaterialData = GetDynamicMaterialData(ParticleIndex);

		FVector2D PositionArray[4];
		PositionArray[0] = FastRotate(FVector2D(-ParticleHalfSize.X, -ParticleHalfSize.Y), ParticleRotationSin, ParticleRotationCos);
		PositionArray[1] = FastRotate(FVector2D(ParticleHalfSize.X, -ParticleHalfSize.Y), ParticleRotationSin, ParticleRotationCos);
		PositionArray[2] = - PositionArray[1];
		PositionArray[3] = - PositionArray[0];
		
		const int VertexIndex = ParticleIndex * 4;
		const int indexIndex = ParticleIndex * 6;		
		
		
		for (int i = 0; i < 4; ++i)
		{
			VertexData[VertexIndex + i].Position = PositionArray[i] + ParticlePosition;
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

void UNiagaraUIComponent::AddRibbonRendererData(SNiagaraUISystemWidget* NiagaraWidget, TSharedRef<const FNiagaraEmitterInstance, ESPMode::ThreadSafe> EmitterInst, UNiagaraRibbonRendererProperties* RibbonRenderer, float ScaleFactor, FVector2D ParentTopLeft, const FNiagaraWidgetProperties* WidgetProperties)
{
	SCOPE_CYCLE_COUNTER(STAT_GenerateRibbonData);
	
	FVector ComponentLocation = GetRelativeLocation();
	FVector ComponentScale = GetRelativeScale3D();
	FRotator ComponentRotation = GetRelativeRotation();

	FNiagaraDataSet& DataSet = EmitterInst->GetData();
	FNiagaraDataBuffer& ParticleData = DataSet.GetCurrentDataChecked();
	const int32 ParticleCount = ParticleData.GetNumInstances();

	if (ParticleCount < 2)
		return;
	

	const auto SortKeyReader = RibbonRenderer->SortKeyDataSetAccessor.GetReader(DataSet);

	const auto PositionData		= RibbonRenderer->PositionDataSetAccessor.GetReader(DataSet);
	const auto ColorData		= FNiagaraDataSetAccessor<FLinearColor>::CreateReader(DataSet, RibbonRenderer->ColorBinding.GetDataSetBindableVariable().GetName());
	const auto RibbonWidthData	= RibbonRenderer->SizeDataSetAccessor.GetReader(DataSet);
	
	const auto RibbonFullIDData = RibbonRenderer->RibbonFullIDDataSetAccessor.GetReader(DataSet);

	auto GetParticlePosition2D = [&PositionData](int32 Index)
	{
		const FVector Position3D = PositionData.GetSafe(Index, FVector::ZeroVector);
		return FVector2D(Position3D.X, -Position3D.Z);
	};	

	auto GetParticleColor = [&ColorData](int32 Index)
	{
		return ColorData.GetSafe(Index, FLinearColor::White);
	};
	
	auto GetParticleWidth = [&RibbonWidthData](int32 Index)
	{
		return RibbonWidthData.GetSafe(Index, 0.f);
	};

	const bool LocalSpace = EmitterInst->GetCachedEmitter()->bLocalSpace;
	const bool FullIDs = RibbonFullIDData.IsValid();
	const bool MultiRibbons = FullIDs;

	auto AddRibbonVerts = [&](TArray<int32>& RibbonIndices)
	{
		const int32 numParticlesInRibbon = RibbonIndices.Num();
		if (numParticlesInRibbon < 3)
			return;
		
		FSlateVertex* VertexData;	
		SlateIndex* IndexData;
	
		FSlateBrush Brush;
		UMaterialInterface* SpriteMaterial = RibbonRenderer->Material;
		
		NiagaraWidget->AddRenderData(&VertexData, &IndexData, SpriteMaterial, (numParticlesInRibbon - 1) * 2, (numParticlesInRibbon - 2) * 6);

		int32 CurrentVertexIndex = 0;
		int32 CurrentIndexIndex = 0;
		
			
		const int32 StartDataIndex = RibbonIndices[0];

		float TotalDistance = 0.0f;

		FVector2D LastPosition = GetParticlePosition2D(StartDataIndex);
		FVector2D CurrentPosition = FVector2D::ZeroVector;
		float CurrentWidth = 0.f;
		FVector2D LastToCurrentVector = FVector2D::ZeroVector;
		float LastToCurrentSize = 0.f;
		float LastU0 = 0.f;
		float LastU1 = 0.f;
		
		FVector2D LastParticleUIPosition = LastPosition * ScaleFactor;
		
		if (LocalSpace)
		{
			LastParticleUIPosition *= FVector2D(ComponentScale.X, ComponentScale.Z);
			LastParticleUIPosition = LastParticleUIPosition.GetRotated(-ComponentRotation.Pitch);
			LastParticleUIPosition +=  ParentTopLeft;
			
			LastParticleUIPosition += FVector2D(ComponentLocation.X, -ComponentLocation.Z) * ScaleFactor;
		}
		else
		{
			LastParticleUIPosition +=  ParentTopLeft;
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
		
		FVector2D InitialPositionArray[2];
		InitialPositionArray[0] = LastToCurrentVector.GetRotated(90.f) * InitialWidth * 0.5f;
		InitialPositionArray[1] = -InitialPositionArray[0];
		
		for (int i = 0; i < 2; ++i)
		{
			VertexData[CurrentVertexIndex + i].Position = InitialPositionArray[i] +  LastParticleUIPosition;
			VertexData[CurrentVertexIndex + i].Color = InitialColor;
			VertexData[CurrentVertexIndex + i].TexCoords[0] = i;
			VertexData[CurrentVertexIndex + i].TexCoords[1] = 0;
		}

		CurrentVertexIndex += 2;

		int32 NextIndex = CurrentIndex + 1;
		
		while (NextIndex < numParticlesInRibbon)
		{
			const int32 NextDataIndex = RibbonIndices[NextIndex];
			const FVector2D NextPosition = GetParticlePosition2D(NextDataIndex);	
			FVector2D CurrentToNextVector = NextPosition - CurrentPosition;
			const float CurrentToNextSize = CurrentToNextVector.Size();		
			CurrentWidth = GetParticleWidth(CurrentDataIndex) * ScaleFactor;
			FColor CurrentColor = GetParticleColor(CurrentDataIndex).ToFColor(true);

			// Normalize CurrToNextVec
			CurrentToNextVector *= 1.f / CurrentToNextSize;
			
			const FVector2D CurrentTangent = (LastToCurrentVector + CurrentToNextVector).GetSafeNormal();
			
			TotalDistance += LastToCurrentSize;

			FVector2D CurrentPositionArray[2];
			CurrentPositionArray[0] = CurrentTangent.GetRotated(90.f) * CurrentWidth * 0.5f;
			CurrentPositionArray[1] = -CurrentPositionArray[0];

			FVector2D CurrentParticleUIPosition = CurrentPosition * ScaleFactor;

			if (LocalSpace)
			{
				CurrentParticleUIPosition *= FVector2D(ComponentScale.X, ComponentScale.Z);
				CurrentParticleUIPosition = CurrentParticleUIPosition.GetRotated(-ComponentRotation.Pitch);
				CurrentParticleUIPosition +=  ParentTopLeft;
				CurrentParticleUIPosition += FVector2D(ComponentLocation.X, -ComponentLocation.Z) * ScaleFactor;
			}
			else
			{
				CurrentParticleUIPosition +=  ParentTopLeft;
			}

			float CurrentU0 = 0.f;
		
			if (RibbonRenderer->UV0Settings.DistributionMode == ENiagaraRibbonUVDistributionMode::TiledOverRibbonLength)
			{
				CurrentU0 = LastU0 + LastToCurrentSize / RibbonRenderer->UV0Settings.TilingLength;
			}
			else
			{
				CurrentU0 = (float)CurrentIndex / (float)numParticlesInRibbon;
			}
			
			float CurrentU1 = 0.f;
		
			if (RibbonRenderer->UV1Settings.DistributionMode == ENiagaraRibbonUVDistributionMode::TiledOverRibbonLength)
			{
				CurrentU1 = LastU1 + LastToCurrentSize / RibbonRenderer->UV1Settings.TilingLength;
			}
			else
			{
				CurrentU1 = (float)CurrentIndex / (float)numParticlesInRibbon;
			}

			FVector2D TextureCoordinates0[2];
			TextureCoordinates0[0] = FVector2D(CurrentU0, 1.f);
			TextureCoordinates0[1] = FVector2D(CurrentU0, 0.f);
			
            FVector2D TextureCoordinates1[2];
			TextureCoordinates1[0] = FVector2D(CurrentU1, 1.f);
			TextureCoordinates1[1] = FVector2D(CurrentU1, 0.f);
			
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

		SortedIndices.Sort([&SortKeyReader](const int32& A, const int32& B) {	return (SortKeyReader[A] < SortKeyReader[B]); });

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
				SortedIndices.Sort([&SortKeyReader](const int32& A, const int32& B) {	return (SortKeyReader[A] < SortKeyReader[B]); });
				AddRibbonVerts(SortedIndices);
			};
		}
	}

}

//PRAGMA_ENABLE_OPTIMIZATION
