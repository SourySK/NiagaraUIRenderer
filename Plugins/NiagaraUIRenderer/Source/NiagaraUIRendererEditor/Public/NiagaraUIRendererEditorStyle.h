// Copyright 2023 Michal Smoleň 

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Styling/SlateTypes.h"

#if ENGINE_MINOR_VERSION < 1
#include "Classes/EditorStyleSettings.h"
#endif

#include "Styling/SlateStyle.h"

/**
 * 
 */
class NIAGARAUIRENDEREREDITOR_API FNiagaraUIRendererEditorStyle
{
public:
	static void Initialize();

	static void Shutdown();

	static TSharedPtr<class ISlateStyle> Get();

	static FName GetStyleSetName();

	static const FSlateBrush* GetBrush( FName PropertyName, const ANSICHAR* Specifier = NULL )
	{
		return StyleSet->GetBrush( PropertyName, Specifier);
	}

	const static FColor WarningBoxComplementaryColor;
	
private:
	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);
	
	class FStyle : public FSlateStyleSet
	{
	public:
		FStyle(const FName& InStyleSetName);
		
		FButtonStyle IgnoreButton;
		FButtonStyle IgnoreButtonFix;
		FButtonStyle AutoPopulateButton;
		FTextBlockStyle IgnoreButtonText;
	};
	
	static TSharedPtr<FNiagaraUIRendererEditorStyle::FStyle> StyleSet;
};
