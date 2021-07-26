// Copyright 2021 - Michal Smoleň

#include "NiagaraUIRendererEditor.h"

#include "NiagaraUIContentBrowserExtension.h"
#include "NiagaraUIRendererEditorStyle.h"
#include "NiagaraWidgetDetailCustomization.h"

#define LOCTEXT_NAMESPACE "FNiagaraUIRendererEditorModule"

void FNiagaraUIRendererEditorModule::StartupModule()
{
	FNiagaraUIRendererEditorStyle::Initialize();
	
	// Details panel	
	FPropertyEditorModule& propertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	propertyModule.RegisterCustomClassLayout("NiagaraSystemWidget", FOnGetDetailCustomizationInstance::CreateStatic(&FNiagaraWidgetDetailCustomization::MakeInstance));
	propertyModule.NotifyCustomizationModuleChanged();

	// Content browser extensions
	if (!IsRunningCommandlet())
	{
		FNiagaraUIContentBrowserExtension::InstallHooks();
	}
}

void FNiagaraUIRendererEditorModule::ShutdownModule()
{
	if(FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		PropertyModule.UnregisterCustomClassLayout("NiagaraSystemWidget");
	}

	// Content browser extensions
	if (!IsRunningCommandlet())
	{
		FNiagaraUIContentBrowserExtension::RemoveHooks();
	}

	FNiagaraUIRendererEditorStyle::Shutdown();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNiagaraUIRendererEditorModule, NiagaraUIRendererEditor)