
#pragma once

#include "Modules/ModuleManager.h"
#include "ExSequence/ExSequence.h"
#include "ExSequence/ExSequenceAgent.h"
#include "BlueprintEditorModule.h"
#include "BlueprintEditorTabs.h"
#include "ExSequence/ExSequenceAgentCustomization.h"
#include "ExSequence/MovieSceneSequenceEditor_ExSequence.h"
#include "ExSequence/ExSequenceEditorStyle.h"
#include "ExSequence/ExSequenceEditorTabSummoner.h"
#include "Framework/Docking/LayoutExtender.h"
#include "LevelEditor.h"
#include "MovieSceneToolsProjectSettings.h"
#include "PropertyEditorModule.h"
#include "Styling/SlateStyle.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "ISettingsModule.h"
#include "SequencerSettings.h"
#include "ISequencerModule.h"


/** Shared class type that ensures safe binding to RegisterBlueprintEditorTab through an SP binding without interfering with module ownership semantics */
class FExSequenceEditorTabBinding
	: public TSharedFromThis<FExSequenceEditorTabBinding>
{
public:

	FExSequenceEditorTabBinding()
	{
		FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");
		BlueprintEditorTabSpawnerHandle = BlueprintEditorModule.OnRegisterTabsForEditor().AddRaw(this, &FExSequenceEditorTabBinding::RegisterBlueprintEditorTab);
		BlueprintEditorLayoutExtensionHandle = BlueprintEditorModule.OnRegisterLayoutExtensions().AddRaw(this, &FExSequenceEditorTabBinding::RegisterBlueprintEditorLayout);

		FLevelEditorModule& LevelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorTabSpawnerHandle = LevelEditor.OnRegisterTabs().AddRaw(this, &FExSequenceEditorTabBinding::RegisterLevelEditorTab);
		LevelEditorLayoutExtensionHandle = LevelEditor.OnRegisterLayoutExtensions().AddRaw(this, &FExSequenceEditorTabBinding::RegisterLevelEditorLayout);
	}

	void RegisterLevelEditorLayout(FLayoutExtender& Extender)
	{
		Extender.ExtendLayout(FTabId("ContentBrowserTab1"), ELayoutExtensionPosition::Before, FTabManager::FTab(FName("EmbeddedExSequenceID"), ETabState::ClosedTab));
	}

	void RegisterBlueprintEditorLayout(FLayoutExtender& Extender)
	{
		Extender.ExtendLayout(FBlueprintEditorTabs::CompilerResultsID, ELayoutExtensionPosition::Before, FTabManager::FTab(FName("EmbeddedExSequenceID"), ETabState::ClosedTab));
	}

	void RegisterBlueprintEditorTab(FWorkflowAllowedTabSet& TabFactories, FName InModeName, TSharedPtr<FBlueprintEditor> BlueprintEditor)
	{
		TabFactories.RegisterFactory(MakeShared<FExSequenceEditorSummoner>(BlueprintEditor));
	}

	void RegisterLevelEditorTab(TSharedPtr<FTabManager> InTabManager)
	{
		auto SpawnTab = [](const FSpawnTabArgs&) -> TSharedRef<SDockTab>
		{
			TSharedRef<SExSequenceEditorWidget> Widget = SNew(SExSequenceEditorWidget, nullptr);

			return SNew(SDockTab)
				.Label(&Widget.Get(), &SExSequenceEditorWidget::GetDisplayLabel)
				.Icon(FExSequenceEditorStyle::Get().GetBrush("ClassIcon.ExSequence"))
				[
					Widget
				];
		};

		InTabManager->RegisterTabSpawner("EmbeddedExSequenceID", FOnSpawnTab::CreateStatic(SpawnTab))
			.SetMenuType(ETabSpawnerMenuType::Hidden)
			.SetAutoGenerateMenuEntry(false);
	}

	~FExSequenceEditorTabBinding()
	{
		FBlueprintEditorModule* BlueprintEditorModule = FModuleManager::GetModulePtr<FBlueprintEditorModule>("Kismet");
		if (BlueprintEditorModule)
		{
			BlueprintEditorModule->OnRegisterTabsForEditor().Remove(BlueprintEditorTabSpawnerHandle);
			BlueprintEditorModule->OnRegisterLayoutExtensions().Remove(BlueprintEditorLayoutExtensionHandle);
		}

		FLevelEditorModule* LevelEditor = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor");
		if (LevelEditor)
		{
			LevelEditor->OnRegisterTabs().Remove(LevelEditorTabSpawnerHandle);
			LevelEditor->OnRegisterLayoutExtensions().Remove(LevelEditorLayoutExtensionHandle);
		}
	}

private:

	/** Delegate binding handle for FBlueprintEditorModule::OnRegisterTabsForEditor */
	FDelegateHandle BlueprintEditorTabSpawnerHandle, BlueprintEditorLayoutExtensionHandle;

	FDelegateHandle LevelEditorTabSpawnerHandle, LevelEditorLayoutExtensionHandle;
};


class FExKismetEditorModule : public IModuleInterface, public FGCObject
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static void OnInitializeSequence(UExSequence* Sequence);

	/** Register details view customizations. */
	void RegisterCustomizations();

	/** Unregister details view customizations. */
	void UnregisterCustomizations();

	/** Register settings objects. */
	void RegisterSettings();

	/** Unregister settings objects. */
	void UnregisterSettings();

	/** FGCObject interface */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

private:

	void AddToolbarExtensionForBlueprintEditor(FToolBarBuilder& Builder);

	/** Callback to keep track of when an asset is opened. */
	static void OnAssetOpenedInEditor(UObject* Object, IAssetEditorInstance* Instance);
	static void OnAssetEditorOpened(UObject* Object);
	/** Called when editor may need to be closed */
	static void OnAssetEditorRequestClose(UObject* Asset, EAssetEditorCloseReason CloseReason);

private:

	FDelegateHandle SequenceEditorHandle;
	FDelegateHandle OnInitializeSequenceHandle;
	TSharedPtr<FExSequenceEditorTabBinding> BlueprintEditorTabBinding;
	FName ExSequenceAgentName;
	USequencerSettings* Settings;

	TSharedPtr<class FUICommandList> ExSequenceUICommandList;
	FDelegateHandle DelegateHandleOnAssetOpenedInEditor;
	FDelegateHandle DelegateHandleOnAssetEditorOpened;
	FDelegateHandle DelegateHandleOnAssetEditorRequestClose;
};
