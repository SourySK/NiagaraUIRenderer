// Copyright 2023 Michal Smoleň 

#include "NiagaraUIRendererEditorStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/CoreStyle.h"

#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush(FNiagaraUIRendererEditorStyle::InContent(RelativePath, ".png"), __VA_ARGS__)
#define BOX_PLUGIN_BRUSH( RelativePath, ... ) FSlateBoxBrush(FNiagaraUIRendererEditorStyle::InContent(RelativePath, ".png"), __VA_ARGS__)
#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

TSharedPtr<FNiagaraUIRendererEditorStyle::FStyle> FNiagaraUIRendererEditorStyle::StyleSet = nullptr;

const FColor FNiagaraUIRendererEditorStyle::WarningBoxComplementaryColor = FColor(232, 63 ,69);

FString FNiagaraUIRendererEditorStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("NiagaraUIRenderer"))->GetContentDir();
	return (ContentDir / RelativePath) + Extension;
}

FNiagaraUIRendererEditorStyle::FStyle::FStyle(const FName& InStyleSetName) : FSlateStyleSet(InStyleSetName)
{
}

void FNiagaraUIRendererEditorStyle::Initialize()
{	
	// Const icon sizes
	const FVector2D Icon8x8(8.0f, 8.0f);
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon25x25(25.0f, 25.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);
	
	if (StyleSet.IsValid())
		return;

	StyleSet = MakeShareable(new FNiagaraUIRendererEditorStyle::FStyle(GetStyleSetName()));
	StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	FNiagaraUIRendererEditorStyle::FStyle& Style = *StyleSet.Get();
	
	{
		Style.Set("ClassIcon.NiagaraSystemWidget", new IMAGE_PLUGIN_BRUSH("Editor/Icons/ParticleIcon", Icon16x16));
		Style.Set("NiagaraUIRendererEditorStyle.ParticleIcon", new IMAGE_PLUGIN_BRUSH("Editor/Icons/ParticleIcon_CB", Icon16x16));
		Style.Set("NiagaraUIRendererEditorStyle.WarningBox.WarningIcon", new IMAGE_PLUGIN_BRUSH("Editor/Icons/NiagaraWarningBox_WarningIcon", Icon25x25));
		Style.Set("NiagaraUIRendererEditorStyle.WarningBox.Refresh", new IMAGE_PLUGIN_BRUSH("Editor/Icons/NiagaraWarningBox_Refresh", Icon20x20));
		Style.Set("NiagaraUIRendererEditorStyle.WarningBox.Bottom", new BOX_PLUGIN_BRUSH("Editor/Icons/NiagaraWarningBox_RoundBottom", FMargin(16/256.0f)));
		Style.Set("NiagaraUIRendererEditorStyle.WarningBox.Top", new BOX_PLUGIN_BRUSH("Editor/Icons/NiagaraWarningBox_RoundTop", FMargin(16/256.0f)));
	}

	Style.IgnoreButton = FButtonStyle()
			.SetNormal(BOX_PLUGIN_BRUSH("Editor/Icons/NiagaraWarningBox_IgnoreButton", FMargin(16/256.0f), FLinearColor(WarningBoxComplementaryColor)))
			.SetHovered(BOX_PLUGIN_BRUSH("Editor/Icons/NiagaraWarningBox_IgnoreButton", FMargin(16/256.0f), FLinearColor(FColor(232, 63 ,69, 175))))
			.SetPressed(BOX_PLUGIN_BRUSH("Editor/Icons/NiagaraWarningBox_IgnoreButton", FMargin(16/256.0f), FLinearColor(FColor(60, 60 ,60, 255))))
			.SetNormalPadding(FMargin(0,0,0,0))
			.SetPressedPadding(FMargin(0,0,0,0));
	
	Style.IgnoreButtonFix = FButtonStyle()
			.SetNormal(BOX_PLUGIN_BRUSH("Editor/Icons/NiagaraWarningBox_IgnoreButtonFix", FMargin(16/256.0f), FLinearColor(WarningBoxComplementaryColor)))
			.SetHovered(BOX_PLUGIN_BRUSH("Editor/Icons/NiagaraWarningBox_IgnoreButtonFix", FMargin(16/256.0f), FLinearColor(FColor(232, 63 ,69, 175))))
			.SetPressed(BOX_PLUGIN_BRUSH("Editor/Icons/NiagaraWarningBox_IgnoreButtonFix", FMargin(16/256.0f), FLinearColor(FColor(60, 60 ,60, 255))))
			.SetNormalPadding(FMargin(0,0,0,0))
			.SetPressedPadding(FMargin(0,0,0,0));
	
	Style.AutoPopulateButton = FButtonStyle()
			.SetNormal(BOX_PLUGIN_BRUSH("Editor/Icons/NiagaraWarningBox_AutoPopulateButton", FMargin(16/256.0f), FLinearColor(FColor(60, 60, 60))))
			.SetHovered(BOX_PLUGIN_BRUSH("Editor/Icons/NiagaraWarningBox_AutoPopulateButton", FMargin(16/256.0f), FLinearColor(FColor(60, 60, 60, 125))))
			.SetPressed(BOX_PLUGIN_BRUSH("Editor/Icons/NiagaraWarningBox_AutoPopulateButton", FMargin(16/256.0f), FLinearColor(FColor(80, 80, 80))))
			.SetNormalPadding(FMargin(0,0,0,0))
			.SetPressedPadding(FMargin(0,0,0,0));
	
	Style.Set("NiagaraUIRendererEditorStyle.WarningBox.IgnoreButton", Style.IgnoreButton);
	Style.Set("NiagaraUIRendererEditorStyle.WarningBox.IgnoreButtonFix", Style.IgnoreButtonFix);
	Style.Set("NiagaraUIRendererEditorStyle.WarningBox.AutoPopulateButton", Style.AutoPopulateButton);

	Style.Set("NiagaraUIRendererEditorStyle.IgnoreButtonText", FTextBlockStyle(Style.IgnoreButtonText)
			.SetFont(DEFAULT_FONT("Bold", 10))
			.SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f)));

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}

void FNiagaraUIRendererEditorStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

TSharedPtr<ISlateStyle> FNiagaraUIRendererEditorStyle::Get()
{
	return StyleSet;
}

FName FNiagaraUIRendererEditorStyle::GetStyleSetName()
{
	static FName NiagaraUIRendererStyle(TEXT("NiagaraUIRendererEditorStyle"));
	return NiagaraUIRendererStyle;
}

#undef IMAGE_PLUGIN_BRUSH