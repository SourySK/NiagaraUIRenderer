// Copyright 2023 Michal Smoleň 


#include "NiagaraUIContentBrowserExtension.h"
#include "ContentBrowserDelegates.h"
#include "ContentBrowserModule.h"
#include "NiagaraUIRendererEditorStyle.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "MaterialEditingLibrary.h"
#include "MaterialGraph/MaterialGraph.h"
#include "Materials/MaterialExpressionVertexColor.h"
#include "Materials/MaterialExpressionParticleColor.h"
#include "IContentBrowserSingleton.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#if ENGINE_MINOR_VERSION >= 2
#include "MaterialDomain.h"
#endif

#define LOCTEXT_NAMESPACE "NiagaraUIRenderer"


static FContentBrowserMenuExtender_SelectedAssets ContentBrowserExtenderDelegate;
static FDelegateHandle ContentBrowserExtenderDelegateHandle;


struct FContentBrowserSelectedAssetExtensionBase
{
public:
	TArray<struct FAssetData> SelectedAssets;

public:
	virtual void Execute() {}
	virtual ~FContentBrowserSelectedAssetExtensionBase() {}
};

#include "IAssetTools.h"
#include "AssetToolsModule.h"

struct FCreateNiagaraUIMaterialsExtension : public FContentBrowserSelectedAssetExtensionBase
{
	void CreateNiagaraUIMaterials(TArray<UMaterial*>& Materials)
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		
		const FString DefaultSuffix = TEXT("_UI");

		TArray<UObject*> NewMaterials;
		
		for (const auto Material : Materials)
		{
			FString Name;
			FString PackageName;
			AssetToolsModule.Get().CreateUniqueAssetName(Material->GetOutermost()->GetName(), DefaultSuffix, PackageName, Name);

			FString TargetPackagePath = FPackageName::GetLongPackagePath(PackageName);

			UObject* DuplicatedObject = AssetToolsModule.Get().DuplicateAsset(Name, TargetPackagePath, Material);

			
			UMaterial* NewMaterial = Cast<UMaterial>(DuplicatedObject);

			if (!NewMaterial)
				return;

			NewMaterials.Add(NewMaterial);

			NewMaterial->MaterialDomain = EMaterialDomain::MD_UI;


#if ENGINE_MINOR_VERSION < 1
			for (UMaterialExpression* Expression : NewMaterial->Expressions)
#else
			for (UMaterialExpression* Expression : NewMaterial->GetExpressions())
#endif
			{
				//if (UMaterialExpressionParticleColor* ParticleColor = Cast<UMaterialExpressionParticleColor>(Expression))
				//if (Expression->IsA(UMaterialExpressionParticleColor::StaticClass()))
				
				if (Expression->GetKeywords().ToString() == TEXT("particle color"))
				{
					UMaterialExpressionVertexColor* VertexColor = Cast<UMaterialExpressionVertexColor>(
						UMaterialEditingLibrary::CreateMaterialExpression(NewMaterial, UMaterialExpressionVertexColor::StaticClass(), Expression->MaterialExpressionEditorX, Expression->MaterialExpressionEditorY));

					for (int32 InputIndex = 0; InputIndex < MP_MAX; InputIndex++)
					{
						FExpressionInput* Input = NewMaterial->GetExpressionInputForProperty((EMaterialProperty)InputIndex);
						if (Input && Input->Expression == Expression)
						{
							Input->Expression = VertexColor;
						}
					}

					FExpressionInput* EmissiveInput = NewMaterial->GetExpressionInputForProperty(EMaterialProperty::MP_EmissiveColor);
					FExpressionInput* BaseInput = NewMaterial->GetExpressionInputForProperty(EMaterialProperty::MP_BaseColor);

					if (EmissiveInput->Expression == nullptr && BaseInput->Expression)
						EmissiveInput->Expression = BaseInput->Expression;

#if ENGINE_MINOR_VERSION < 1
					for (UMaterialExpression* TestExp : NewMaterial->Expressions)
#else
					for (UMaterialExpression* TestExp : NewMaterial->GetExpressions())
#endif
					{
						TArrayView<FExpressionInput*> Inputs = TestExp->GetInputsView();

						for (FExpressionInput* Input : Inputs)
						{
							if (Input->Expression == Expression)
							{
								Input->Expression = VertexColor;
							}
						}
					}

					UMaterialEditingLibrary::DeleteMaterialExpression(NewMaterial, Expression);
				}
			}
			

			NewMaterial->PreEditChange(nullptr);
			NewMaterial->PostEditChange();
			NewMaterial->Modify();
		}

		if (NewMaterials.Num() == 1)
		{			
			ContentBrowserModule.Get().SyncBrowserToAssets(NewMaterials);
			
			// TODO: Automatically rename
		}
		else if (NewMaterials.Num() > 1)
		{
			ContentBrowserModule.Get().SyncBrowserToAssets(NewMaterials);
		}
	}

	virtual void Execute() override
	{
		TArray<UMaterial*> Materials;
		for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
		{
			const FAssetData& AssetData = *AssetIt;
			if (UMaterial* Material = Cast<UMaterial>(AssetData.GetAsset()))
			{
				Materials.Add(Material);
			}
		}

		CreateNiagaraUIMaterials(Materials);
	}
};

class FNiagaraUIContentBrowserExtension_Impl
{
public:
	static void ExecuteSelectedContentFunctor(TSharedPtr<FContentBrowserSelectedAssetExtensionBase> SelectedAssetFunctor)
	{
		SelectedAssetFunctor->Execute();
	}

	static void CreateNiagaraUIFunctions(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
	{
		TArray<UMaterial*> Materials;
		for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
		{
			const FAssetData& AssetData = *AssetIt;
			if (UMaterial* Material = Cast<UMaterial>(AssetData.GetAsset()))
			{
				Materials.Add(Material);
			}
		}

		TSharedPtr<FCreateNiagaraUIMaterialsExtension> NiagaraUIMaterialsFunctor = MakeShareable(new FCreateNiagaraUIMaterialsExtension());
		NiagaraUIMaterialsFunctor->SelectedAssets = SelectedAssets;

		FUIAction ActionCreateNiagaraUIMaterial(FExecuteAction::CreateStatic(&FNiagaraUIContentBrowserExtension_Impl::ExecuteSelectedContentFunctor, StaticCastSharedPtr<FContentBrowserSelectedAssetExtensionBase>(NiagaraUIMaterialsFunctor)));
		
		MenuBuilder.AddMenuEntry(
			LOCTEXT("NiagaraUIRenderer_CreateUIMaterial", "Create Niagara UI Material"),
			LOCTEXT("NiagaraUIRenderer_CreateUIMaterialTooltip", "Creates a new version of this material that is suitable to be used for the Niagara UI Particles. Be aware that manual inspection of this material is recommended."),
			FSlateIcon(FNiagaraUIRendererEditorStyle::GetStyleSetName(), "NiagaraUIRendererEditorStyle.ParticleIcon"),
			ActionCreateNiagaraUIMaterial,
			NAME_None,
			EUserInterfaceActionType::Button
		);
	}

	static TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
	{
		TSharedRef<FExtender> Extender(new FExtender());

		bool AnyMaterials = false;
		for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
		{
			const FAssetData& Asset = *AssetIt;
			
#if ENGINE_MINOR_VERSION < 1
			AnyMaterials = AnyMaterials || (Asset.AssetClass == UMaterial::StaticClass()->GetFName());
#else
			AnyMaterials = AnyMaterials || (Asset.AssetClassPath == UMaterial::StaticClass()->GetClassPathName());
#endif
			
		}

		if (AnyMaterials)
		{
			Extender->AddMenuExtension(
				"GetAssetActions",
				EExtensionHook::After,
				nullptr,
				FMenuExtensionDelegate::CreateStatic(&FNiagaraUIContentBrowserExtension_Impl::CreateNiagaraUIFunctions, SelectedAssets));
		}

		return Extender;
	}

	static TArray<FContentBrowserMenuExtender_SelectedAssets>& GetExtenderDelegates()
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		return ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	}
};

void FNiagaraUIContentBrowserExtension::InstallHooks()
{
	ContentBrowserExtenderDelegate = FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&FNiagaraUIContentBrowserExtension_Impl::OnExtendContentBrowserAssetSelectionMenu);

	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = FNiagaraUIContentBrowserExtension_Impl::GetExtenderDelegates();
	CBMenuExtenderDelegates.Add(ContentBrowserExtenderDelegate);
	ContentBrowserExtenderDelegateHandle = CBMenuExtenderDelegates.Last().GetHandle();
}

void FNiagaraUIContentBrowserExtension::RemoveHooks()
{
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = FNiagaraUIContentBrowserExtension_Impl::GetExtenderDelegates();
	CBMenuExtenderDelegates.RemoveAll([](const FContentBrowserMenuExtender_SelectedAssets& Delegate){ return Delegate.GetHandle() == ContentBrowserExtenderDelegateHandle; });
}

#undef LOCTEXT_NAMESPACE