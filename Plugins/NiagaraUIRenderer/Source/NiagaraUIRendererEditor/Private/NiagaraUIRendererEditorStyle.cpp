// Copyright 2024 - Michal Smoleň

#include "NiagaraUIRendererEditorStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateStyleMacros.h"

TSharedPtr<FNiagaraUIRendererEditorStyle::FStyle> FNiagaraUIRendererEditorStyle::StyleSet = nullptr;

const FColor FNiagaraUIRendererEditorStyle::WarningBoxComplementaryColor = FColor(232, 63 ,69);

FNiagaraUIRendererEditorStyle::FStyle::FStyle(const FName& InStyleSetName) : FSlateStyleSet(InStyleSetName)
{
}

void FNiagaraUIRendererEditorStyle::Initialize()
{
	if (StyleSet.IsValid())
		return;

	StyleSet = MakeShareable(new FNiagaraUIRendererEditorStyle::FStyle(GetStyleSetName()));
	StyleSet->Initialize();
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

void FNiagaraUIRendererEditorStyle::FStyle::Initialize()
{	
	const FString PluginContentDir = IPluginManager::Get().FindPlugin(TEXT("NiagaraUIRenderer"))->GetContentDir();
	SetContentRoot(PluginContentDir);
	SetCoreContentRoot(PluginContentDir);

	{
		Set("ClassIcon.NiagaraSystemWidget", new IMAGE_BRUSH("Editor/Icons/ParticleIcon", CoreStyleConstants::Icon16x16));
		Set("NiagaraUIRendererEditorStyle.ParticleIcon", new IMAGE_BRUSH("Editor/Icons/ParticleIcon_CB", CoreStyleConstants::Icon16x16));
		Set("NiagaraUIRendererEditorStyle.WarningBox.WarningIcon", new IMAGE_BRUSH("Editor/Icons/NiagaraWarningBox_WarningIcon", CoreStyleConstants::Icon25x25));
		Set("NiagaraUIRendererEditorStyle.WarningBox.Refresh", new IMAGE_BRUSH("Editor/Icons/NiagaraWarningBox_Refresh", CoreStyleConstants::Icon20x20));
		Set("NiagaraUIRendererEditorStyle.WarningBox.Bottom", new BOX_BRUSH("Editor/Icons/NiagaraWarningBox_RoundBottom", FMargin(16/256.0f)));
		Set("NiagaraUIRendererEditorStyle.WarningBox.Top", new BOX_BRUSH("Editor/Icons/NiagaraWarningBox_RoundTop", FMargin(16/256.0f)));
	}

	IgnoreButton = FButtonStyle()
			.SetNormal(BOX_BRUSH("Editor/Icons/NiagaraWarningBox_IgnoreButton", FMargin(16/256.0f), FLinearColor(WarningBoxComplementaryColor)))
			.SetHovered(BOX_BRUSH("Editor/Icons/NiagaraWarningBox_IgnoreButton", FMargin(16/256.0f), FLinearColor(FColor(232, 63 ,69, 175))))
			.SetPressed(BOX_BRUSH("Editor/Icons/NiagaraWarningBox_IgnoreButton", FMargin(16/256.0f), FLinearColor(FColor(60, 60 ,60, 255))))
			.SetNormalPadding(FMargin(0,0,0,0))
			.SetPressedPadding(FMargin(0,0,0,0));
	
	IgnoreButtonFix = FButtonStyle()
			.SetNormal(BOX_BRUSH("Editor/Icons/NiagaraWarningBox_IgnoreButtonFix", FMargin(16/256.0f), FLinearColor(WarningBoxComplementaryColor)))
			.SetHovered(BOX_BRUSH("Editor/Icons/NiagaraWarningBox_IgnoreButtonFix", FMargin(16/256.0f), FLinearColor(FColor(232, 63 ,69, 175))))
			.SetPressed(BOX_BRUSH("Editor/Icons/NiagaraWarningBox_IgnoreButtonFix", FMargin(16/256.0f), FLinearColor(FColor(60, 60 ,60, 255))))
			.SetNormalPadding(FMargin(0,0,0,0))
			.SetPressedPadding(FMargin(0,0,0,0));
	
	AutoPopulateButton = FButtonStyle()
			.SetNormal(BOX_BRUSH("Editor/Icons/NiagaraWarningBox_AutoPopulateButton", FMargin(16/256.0f), FLinearColor(FColor(60, 60, 60))))
			.SetHovered(BOX_BRUSH("Editor/Icons/NiagaraWarningBox_AutoPopulateButton", FMargin(16/256.0f), FLinearColor(FColor(60, 60, 60, 125))))
			.SetPressed(BOX_BRUSH("Editor/Icons/NiagaraWarningBox_AutoPopulateButton", FMargin(16/256.0f), FLinearColor(FColor(80, 80, 80))))
			.SetNormalPadding(FMargin(0,0,0,0))
			.SetPressedPadding(FMargin(0,0,0,0));
	
	Set("NiagaraUIRendererEditorStyle.WarningBox.IgnoreButton", IgnoreButton);
	Set("NiagaraUIRendererEditorStyle.WarningBox.IgnoreButtonFix", IgnoreButtonFix);
	Set("NiagaraUIRendererEditorStyle.WarningBox.AutoPopulateButton", AutoPopulateButton);

	Set("NiagaraUIRendererEditorStyle.IgnoreButtonText", FTextBlockStyle(IgnoreButtonText)
			.SetFont(DEFAULT_FONT("Bold", 10))
			.SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f)));
}

