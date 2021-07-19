
#include "ExKismetEditor.h"
#include "BlueprintEditorModule.h"
#include "BlueprintEditor.h"
#include "SKismetInspector.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Editor.h"
#include "PropertyEditorDelegates.h"
#include "Engine/Blueprint.h"
#include "KismetCompilerModule.h"
#include "ExSequence/ExSequenceDetails.h"

#define LOCTEXT_NAMESPACE "FExKismetEditorModule"

void FExKismetEditorModule::StartupModule()
{
	// Register styles
	FExSequenceEditorStyle::Get();

	BlueprintEditorTabBinding = MakeShared<FExSequenceEditorTabBinding>();
	RegisterCustomizations();
	RegisterSettings();
	OnInitializeSequenceHandle = UExSequence::OnInitializeSequence().AddStatic(FExKismetEditorModule::OnInitializeSequence);

	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
	SequenceEditorHandle = SequencerModule.RegisterSequenceEditor(UExSequence::StaticClass(), MakeUnique<FMovieSceneSequenceEditor_ExSequence>());

	{
		TSharedPtr<FExtender> ExtenderOfToolBar = MakeShareable(new FExtender());
		ExtenderOfToolBar->AddToolBarExtension("Asset", EExtensionHook::After, ExSequenceUICommandList, FToolBarExtensionDelegate::CreateRaw(this, &FExKismetEditorModule::AddToolbarExtensionForBlueprintEditor));
		FAssetEditorToolkit::GetSharedToolBarExtensibilityManager()->AddExtender(ExtenderOfToolBar);
	}

	{
		// Register delegates
		if (GEditor)
		{
			DelegateHandleOnAssetOpenedInEditor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OnAssetOpenedInEditor().AddStatic(&FExKismetEditorModule::OnAssetOpenedInEditor);
			DelegateHandleOnAssetEditorOpened = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OnAssetEditorOpened().AddStatic(&FExKismetEditorModule::OnAssetEditorOpened);

			DelegateHandleOnAssetEditorRequestClose = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OnAssetEditorRequestClose().AddStatic(&FExKismetEditorModule::OnAssetEditorRequestClose);
		}
	}
}

void FExKismetEditorModule::ShutdownModule()
{
	{
		//IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
		//KismetCompilerModule.GetCompilers().Remove(&ExBlueprintCompiler);
	}

	UExSequence::OnInitializeSequence().Remove(OnInitializeSequenceHandle);
	UnregisterCustomizations();
	UnregisterSettings();

	ISequencerModule* SequencerModule = FModuleManager::Get().GetModulePtr<ISequencerModule>("Sequencer");
	if (SequencerModule)
	{
		SequencerModule->UnregisterSequenceEditor(SequenceEditorHandle);
	}

	BlueprintEditorTabBinding = nullptr;
	
	{
		if (GEditor)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OnAssetOpenedInEditor().Remove(DelegateHandleOnAssetOpenedInEditor);
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OnAssetEditorOpened().Remove(DelegateHandleOnAssetEditorOpened);

			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OnAssetEditorRequestClose().Remove(DelegateHandleOnAssetEditorRequestClose);
		}
	}
}

void FExKismetEditorModule::OnInitializeSequence(UExSequence* Sequence)
{
	auto* ProjectSettings = GetDefault<UMovieSceneToolsProjectSettings>();
	UMovieScene* MovieScene = Sequence->GetMovieScene();

	FFrameNumber StartFrame = (ProjectSettings->DefaultStartTime * MovieScene->GetTickResolution()).RoundToFrame();
	int32        Duration = (ProjectSettings->DefaultDuration * MovieScene->GetTickResolution()).RoundToFrame().Value;

	MovieScene->SetPlaybackRange(StartFrame, Duration);
}

void FExKismetEditorModule::RegisterCustomizations()
{
	// Register the details customizer
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	{
		ExSequenceAgentName = UExSequenceAgent::StaticClass()->GetFName();
		PropertyModule.RegisterCustomClassLayout(ExSequenceAgentName, FOnGetDetailCustomizationInstance::CreateStatic(&FExSequenceAgentCustomization::MakeInstance));
	}
	
	{
		PropertyModule.RegisterCustomPropertyTypeLayout("ExSequence", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FExSequenceDetails::MakeInstance));
	}
}

void FExKismetEditorModule::UnregisterCustomizations()
{
	FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");
	if (PropertyModule)
	{
		PropertyModule->UnregisterCustomPropertyTypeLayout(ExSequenceAgentName);
	}
}

void FExKismetEditorModule::RegisterSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		Settings = USequencerSettingsContainer::GetOrCreate<USequencerSettings>(TEXT("EmbeddedExSequenceEditor"));

		SettingsModule->RegisterSettings("Editor", "ContentEditors", "EmbeddedExSequenceEditor",
			LOCTEXT("EmbeddedExSequenceEditorSettingsName", "Embedded Ex Sequence Editor"),
			LOCTEXT("EmbeddedExSequenceEditorSettingsDescription", "Configure the look and feel of the Embedded Ex Sequence Editor."),
			Settings);

		Settings->SetCompileDirectorOnEvaluate(false);
		Settings->SetKeepPlayRangeInSectionBounds(false);
	}
}

void FExKismetEditorModule::UnregisterSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		SettingsModule->UnregisterSettings("Editor", "ContentEditors", "EmbeddedExSequenceEditor");
	}
}

void FExKismetEditorModule::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (Settings)
	{
		Collector.AddReferencedObject(Settings);
	}
}

void FExKismetEditorModule::AddToolbarExtensionForBlueprintEditor(FToolBarBuilder& Builder)
{
	// TODO.
}

void FExKismetEditorModule::OnAssetOpenedInEditor(UObject* Object, IAssetEditorInstance* Instance)
{

}

void FExKismetEditorModule::OnAssetEditorOpened(UObject* Object)
{
	
}

void FExKismetEditorModule::OnAssetEditorRequestClose(UObject* Asset, EAssetEditorCloseReason CloseReason)
{
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FExKismetEditorModule, ExKismetEditor)