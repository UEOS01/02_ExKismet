
#include "ExSequence/ExSequenceEditorTabSummoner.h"

#include "ExSequence/ExSequence.h"
#include "ExSequence/ExSequenceAgent.h"
#include "ISequencer.h"
#include "ISequencerModule.h"
#include "LevelEditorSequencerIntegration.h"
#include "SSCSEditor.h"
#include "Styling/SlateIconFinder.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EditorStyleSet.h"
#include "EditorUndoClient.h"
#include "Widgets/Images/SImage.h"
#include "Editor.h"
#include "ScopedTransaction.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Application/SlateApplication.h"
#include "ExSequence/ExSequenceEditorStyle.h"

#define LOCTEXT_NAMESPACE "ExSequenceEditorSummoner"

DECLARE_DELEGATE_OneParam(FExOnComponentSelected, TSharedPtr<FSCSEditorTreeNode>);
DECLARE_DELEGATE_RetVal_OneParam(bool, FExIsComponentValid, UActorComponent*);


class SComponentSelectionTree
	: public SCompoundWidget
{

public:

	SLATE_BEGIN_ARGS(SComponentSelectionTree) : _IsInEditMode(false) {}

		SLATE_EVENT(FExOnComponentSelected, OnComponentSelected)
		SLATE_EVENT(FExIsComponentValid, IsComponentValid)
		SLATE_ARGUMENT(bool, IsInEditMode)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, AActor* InPreviewActor)
	{
		bIsInEditMode = InArgs._IsInEditMode;
		OnComponentSelected = InArgs._OnComponentSelected;
		IsComponentValid = InArgs._IsComponentValid;

		ChildSlot
		[
			SAssignNew(TreeView, STreeView<TSharedPtr<FSCSEditorTreeNode>>)
			.TreeItemsSource(&RootNodes)
			.SelectionMode(ESelectionMode::Single)
			.OnGenerateRow(this, &SComponentSelectionTree::GenerateRow)
			.OnGetChildren(this, &SComponentSelectionTree::OnGetChildNodes)
			.OnSelectionChanged(this, &SComponentSelectionTree::OnSelectionChanged)
			.ItemHeight(24)
		];

		BuildTree(InPreviewActor);

		if (RootNodes.Num() == 0)
		{
			ChildSlot
			[
				SNew(SBox)
				.Padding(FMargin(5.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("NoValidComponentsFound", "No valid components available"))
				]
			];
		}
	}

	void BuildTree(AActor* Actor)
	{
		RootNodes.Reset();
		ObjectToNode.Reset();

		for (UActorComponent* Component : TInlineComponentArray<UActorComponent*>(Actor))
		{
			if (IsComponentVisibleInTree(Component))
			{
				FindOrAddNodeForComponent(Component);
			}
		}
	}

private:

	void OnSelectionChanged(TSharedPtr<FSCSEditorTreeNode> InNode, ESelectInfo::Type SelectInfo)
	{
		OnComponentSelected.ExecuteIfBound(InNode);
	}

	void OnGetChildNodes(TSharedPtr<FSCSEditorTreeNode> InNodePtr, TArray<TSharedPtr<FSCSEditorTreeNode>>& OutChildren)
	{
		OutChildren = InNodePtr->GetChildren();
	}

	TSharedRef<ITableRow> GenerateRow(TSharedPtr<FSCSEditorTreeNode> InNodePtr, const TSharedRef<STableViewBase>& OwnerTable)
	{
		const FSlateBrush* ComponentIcon = FEditorStyle::GetBrush("SCS.NativeComponent");
		if (InNodePtr->GetComponentTemplate() != NULL)
		{
			ComponentIcon = FSlateIconFinder::FindIconBrushForClass( InNodePtr->GetComponentTemplate()->GetClass(), TEXT("SCS.Component") );
		}

		FText Label = InNodePtr->IsInheritedComponent() && !bIsInEditMode
			? FText::Format(LOCTEXT("NativeComponentFormatString","{0} (Inherited)"), FText::FromString(InNodePtr->GetDisplayString()))
			: FText::FromString(InNodePtr->GetDisplayString());

		TSharedRef<STableRow<FSCSEditorTreeNodePtrType>> Row = SNew(STableRow<FSCSEditorTreeNodePtrType>, OwnerTable).Padding(FMargin(0.f, 0.f, 0.f, 4.f));
		Row->SetContent(
			SNew(SHorizontalBox)

			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(ComponentIcon)
				.ColorAndOpacity(SSCS_RowWidget::GetColorTintForIcon(InNodePtr))
			]

			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(2, 0, 0, 0)
			[
				SNew(STextBlock)
				.Text(Label)
			]);

		return Row;
	}

	bool IsComponentVisibleInTree(UActorComponent* ActorComponent) const
	{
		return !IsComponentValid.IsBound() || IsComponentValid.Execute(ActorComponent);
	}

	TSharedPtr<FSCSEditorTreeNode> FindOrAddNodeForComponent(UActorComponent* ActorComponent)
	{
		if (ActorComponent->IsEditorOnly())
		{
			return nullptr;
		}

		if (TSharedPtr<FSCSEditorTreeNode>* Existing = ObjectToNode.Find(ActorComponent))
		{
			return *Existing;
		}
		else if (USceneComponent* SceneComponent = Cast<USceneComponent>(ActorComponent))
		{
			if (UActorComponent* Parent = SceneComponent->GetAttachParent())
			{
				TSharedPtr<FSCSEditorTreeNode> ParentNode = FindOrAddNodeForComponent(Parent);

				if (!ParentNode.IsValid())
				{
					return nullptr;
				}

				TreeView->SetItemExpansion(ParentNode, true);

				TSharedPtr<FSCSEditorTreeNode> ChildNode = ParentNode->AddChildFromComponent(ActorComponent);
				ObjectToNode.Add(ActorComponent, ChildNode);

				return ChildNode;
			}
		}
		
		TSharedPtr<FSCSEditorTreeNode> RootNode = FSCSEditorTreeNode::FactoryNodeFromComponent(ActorComponent);
		RootNodes.Add(RootNode);
		ObjectToNode.Add(ActorComponent, RootNode);

		TreeView->SetItemExpansion(RootNode, true);

		return RootNode;
	}

private:
	bool bIsInEditMode;
	FExOnComponentSelected OnComponentSelected;
	FExIsComponentValid IsComponentValid;
	TSharedPtr<STreeView<TSharedPtr<FSCSEditorTreeNode>>> TreeView;
	TMap<FObjectKey, TSharedPtr<FSCSEditorTreeNode>> ObjectToNode;
	TArray<TSharedPtr<FSCSEditorTreeNode>> RootNodes;
};

class SExSequenceEditorWidgetImpl : public SCompoundWidget, public FEditorUndoClient
{
public:

	SLATE_BEGIN_ARGS(SExSequenceEditorWidgetImpl){}
	SLATE_END_ARGS();

	void Close()
	{
		if (Sequencer.IsValid())
		{
			FLevelEditorSequencerIntegration::Get().RemoveSequencer(Sequencer.ToSharedRef());
			Sequencer->Close();
			Sequencer = nullptr;
		}

		if (WeakBlueprintEditor.IsValid() && WeakBlueprintEditor.Pin()->IsHosted())
		{
			const FName CurveEditorTabName = FName(TEXT("SequencerGraphEditor"));
			TSharedPtr<SDockTab> ExistingTab = WeakBlueprintEditor.Pin()->GetToolkitHost()->GetTabManager()->FindExistingLiveTab(CurveEditorTabName);
			if (ExistingTab)
			{
				ExistingTab->RequestCloseTab();
			}
		}

		GEditor->UnregisterForUndo(this);
		GEditor->OnBlueprintPreCompile().Remove(OnBlueprintPreCompileHandle);
		FCoreUObjectDelegates::OnObjectSaved.Remove(OnObjectSavedHandle);
	}

	~SExSequenceEditorWidgetImpl()
	{
		Close();
	}
	
	TSharedRef<SDockTab> SpawnCurveEditorTab(const FSpawnTabArgs&)
	{
		const FSlateIcon SequencerGraphIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "GenericCurveEditor.TabIcon");
		return SNew(SDockTab)
			.Icon(SequencerGraphIcon.GetIcon())
			.Label(NSLOCTEXT("ExSequencer", "ExSequencerMainGraphEditorTitle", "ExSequencer Curves"))
			[
				SNullWidget::NullWidget
			];
	}

	void Construct(const FArguments&, TWeakPtr<FBlueprintEditor> InBlueprintEditor)
	{
		OnBlueprintPreCompileHandle = GEditor->OnBlueprintPreCompile().AddSP(this, &SExSequenceEditorWidgetImpl::OnBlueprintPreCompile);
		OnObjectSavedHandle = FCoreUObjectDelegates::OnObjectSaved.AddSP(this, &SExSequenceEditorWidgetImpl::OnObjectPreSave);

		WeakBlueprintEditor = InBlueprintEditor;

		{
			const FName CurveEditorTabName = FName(TEXT("SequencerGraphEditor"));
			if (WeakBlueprintEditor.IsValid() && !WeakBlueprintEditor.Pin()->GetTabManager()->HasTabSpawner(CurveEditorTabName))
			{
				// Register an empty tab to spawn the Curve Editor in so that layouts restore properly.
				WeakBlueprintEditor.Pin()->GetTabManager()->RegisterTabSpawner(CurveEditorTabName,
					FOnSpawnTab::CreateSP(this, &SExSequenceEditorWidgetImpl::SpawnCurveEditorTab))
					.SetMenuType(ETabSpawnerMenuType::Type::Hidden);
			}
		}

		ChildSlot
		[
			SAssignNew(Content, SBox)
			.MinDesiredHeight(200)
		];

		GEditor->RegisterForUndo(this);
	}


	virtual void PostUndo(bool bSuccess) override
	{
		if (!GetExSequence())
		{
			Close();
		}
	}

	FText GetDisplayLabel() const
	{
		UExSequence* Sequence = WeakSequence.Get();
		return Sequence ? Sequence->GetDisplayName() : LOCTEXT("DefaultSequencerLabel", "ExSequencer");
	}

	UExSequence* GetExSequence() const
	{
		return WeakSequence.Get();
	}

	UObject* GetPlaybackContext() const
	{
		UExSequence* LocalExSequence = GetExSequence();
		UExSequenceAgent* ExSequenceAgent = nullptr;

		if (LocalExSequence)
		{
			ExSequenceAgent = LocalExSequence->GetTypedOuter<UExSequenceAgent>();
		}

		if (ExSequenceAgent)
		{
			if (AActor* Actor = ExSequenceAgent->GetTypedOuter<AActor>())
			{
				return Actor;
			}
			else if (UBlueprintGeneratedClass* GeneratedClass = ExSequenceAgent->GetTypedOuter<UBlueprintGeneratedClass>())
			{
				if (GeneratedClass->SimpleConstructionScript)
				{
					return GeneratedClass->SimpleConstructionScript->GetComponentEditorActorInstance();
				}
			}
			
			UObject* OuterTemp = ExSequenceAgent->GetOuter();
			return OuterTemp;
		}
		
		return nullptr;
	}

	TArray<UObject*> GetEventContexts() const
	{
		TArray<UObject*> Contexts;
		if (auto* Context = GetPlaybackContext())
		{
			Contexts.Add(Context);
		}
		return Contexts;
	}

	void SetExSequence(UExSequence* NewSequence)
	{
		if (UExSequence* OldSequence = WeakSequence.Get())
		{
			if (OnSequenceChangedHandle.IsValid())
			{
				OldSequence->OnSignatureChanged().Remove(OnSequenceChangedHandle);
			}
		}

		WeakSequence = NewSequence;

		if (NewSequence)
		{
			OnSequenceChangedHandle = NewSequence->OnSignatureChanged().AddSP(this, &SExSequenceEditorWidgetImpl::OnSequenceChanged);
		}

		// If we already have a sequencer open, just assign the sequence
		if (Sequencer.IsValid() && NewSequence)
		{
			if (Sequencer->GetRootMovieSceneSequence() != NewSequence)
			{
				Sequencer->ResetToNewRootSequence(*NewSequence);
			}
			return;
		}

		// If we're setting the sequence to none, destroy sequencer
		if (!NewSequence)
		{
			if (Sequencer.IsValid())
			{
				FLevelEditorSequencerIntegration::Get().RemoveSequencer(Sequencer.ToSharedRef());
				Sequencer->Close();
				Sequencer = nullptr;
			}

			Content->SetContent(SNew(STextBlock).Text(LOCTEXT("NothingSelected", "Select a sequence")));
			return;
		}

		// We need to initialize a new sequencer instance
		FSequencerInitParams SequencerInitParams;
		{
			TWeakObjectPtr<UExSequence> LocalWeakSequence = NewSequence;

			SequencerInitParams.RootSequence = NewSequence;
			SequencerInitParams.EventContexts = TAttribute<TArray<UObject*>>(this, &SExSequenceEditorWidgetImpl::GetEventContexts);
			SequencerInitParams.PlaybackContext = TAttribute<UObject*>(this, &SExSequenceEditorWidgetImpl::GetPlaybackContext);
			
			if (WeakBlueprintEditor.IsValid())
			{
				SequencerInitParams.ToolkitHost = WeakBlueprintEditor.Pin()->GetToolkitHost();
				SequencerInitParams.HostCapabilities.bSupportsCurveEditor = true;
			}

			TSharedRef<FExtender> AddMenuExtender = MakeShareable(new FExtender);

			AddMenuExtender->AddMenuExtension("AddTracks", EExtensionHook::Before, nullptr,
				FMenuExtensionDelegate::CreateLambda([=](FMenuBuilder& MenuBuilder){

					MenuBuilder.AddSubMenu(
						LOCTEXT("AddComponent_Label", "Component"),
						LOCTEXT("AddComponent_ToolTip", "Add a binding to one of this actor's components and allow it to be animated by Sequencer"),
						FNewMenuDelegate::CreateRaw(this, &SExSequenceEditorWidgetImpl::AddPossessComponentMenuExtensions),
						false /*bInOpenSubMenuOnClick*/,
						FSlateIcon()//"LevelSequenceEditorStyle", "LevelSequenceEditor.PossessNewActor")
						);

				})
			);

			SequencerInitParams.ViewParams.bReadOnly = !WeakBlueprintEditor.IsValid() && !NewSequence->IsEditable();
			SequencerInitParams.bEditWithinLevelEditor = false;
			SequencerInitParams.ViewParams.AddMenuExtender = AddMenuExtender;
			SequencerInitParams.ViewParams.UniqueName = "EmbeddedExSequenceEditor";
			SequencerInitParams.ViewParams.ScrubberStyle = ESequencerScrubberStyle::FrameBlock;
			SequencerInitParams.ViewParams.OnReceivedFocus.BindRaw(this, &SExSequenceEditorWidgetImpl::OnSequencerReceivedFocus);
		}

		Sequencer = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer").CreateSequencer(SequencerInitParams);
		Content->SetContent(Sequencer->GetSequencerWidget());

		FLevelEditorSequencerIntegrationOptions Options;
		Options.bRequiresLevelEvents = true;
		Options.bRequiresActorEvents = false;
		Options.bForceRefreshDetails = false;
		Options.bCanRecord = false;

		FLevelEditorSequencerIntegration::Get().AddSequencer(Sequencer.ToSharedRef(), Options);
	}

	void OnSequencerReceivedFocus()
	{
		if (Sequencer.IsValid())
		{
			FLevelEditorSequencerIntegration::Get().OnSequencerReceivedFocus(Sequencer.ToSharedRef());
		}
	}

	void OnObjectPreSave(UObject* InObject)
	{
		TSharedPtr<FBlueprintEditor> BlueprintEditor = WeakBlueprintEditor.Pin();
		if (Sequencer.IsValid() && BlueprintEditor.IsValid() && InObject && InObject == BlueprintEditor->GetBlueprintObj())
		{
			Sequencer->RestorePreAnimatedState();
		}
	}

	void OnBlueprintPreCompile(UBlueprint* InBlueprint)
	{
		TSharedPtr<FBlueprintEditor> BlueprintEditor = WeakBlueprintEditor.Pin();
		if (Sequencer.IsValid() && BlueprintEditor.IsValid() && InBlueprint && InBlueprint == BlueprintEditor->GetBlueprintObj())
		{
			Sequencer->RestorePreAnimatedState();
		}
	}

	void OnSelectionUpdated(TSharedPtr<FSCSEditorTreeNode> SelectedNode)
	{
		if (SelectedNode->GetNodeType() != FSCSEditorTreeNode::ComponentNode)
		{
			return;
		}

		UActorComponent* EditingComponent = nullptr;

		TSharedPtr<FBlueprintEditor> BlueprintEditor = WeakBlueprintEditor.Pin();
		if (BlueprintEditor.IsValid())
		{
			UBlueprint* Blueprint = BlueprintEditor->GetBlueprintObj();
			if (Blueprint)
			{
				EditingComponent = SelectedNode->GetOrCreateEditableComponentTemplate(Blueprint);
			}
		}
		else if (AActor* Actor = GetPreviewActor())
		{
			EditingComponent = SelectedNode->FindComponentInstanceInActor(Actor);
		}

		if (EditingComponent)
		{
			const FScopedTransaction Transaction(LOCTEXT("AddComponentToSequencer", "Add component to Sequencer"));
			Sequencer->GetHandleToObject(EditingComponent, true);
		}

		FSlateApplication::Get().DismissAllMenus();
	}

	void AddPossessComponentMenuExtensions(FMenuBuilder& MenuBuilder)
	{
		AActor* Actor = GetPreviewActor();
		if (!Actor)
		{
			return;
		}

		Sequencer->State.ClearObjectCaches(*Sequencer);
		TSet<UObject*> AllBoundObjects;

		AllBoundObjects.Add(GetOwnerComponent());

		UMovieScene* MovieScene = Sequencer->GetFocusedMovieSceneSequence()->GetMovieScene();
		for (int32 Index = 0; Index < MovieScene->GetPossessableCount(); ++Index)
		{
			FMovieScenePossessable& Possessable = MovieScene->GetPossessable(Index);
			for (TWeakObjectPtr<> WeakObject : Sequencer->FindBoundObjects(Possessable.GetGuid(), Sequencer->GetFocusedTemplateID()))
			{
				if (UObject* Object = WeakObject.Get())
				{
					AllBoundObjects.Add(Object);
				}
			}
		}

		bool bIdent = false;
		MenuBuilder.AddWidget(
			SNew(SComponentSelectionTree, Actor)
			.IsInEditMode(WeakBlueprintEditor.Pin().IsValid())
			.OnComponentSelected(this, &SExSequenceEditorWidgetImpl::OnSelectionUpdated)
			.IsComponentValid_Lambda(
				[AllBoundObjects](UActorComponent* Component)
				{
					return !AllBoundObjects.Contains(Component);
				}
			)
			, FText(), !bIdent
		);
	}

	AActor* GetPreviewActor() const
	{
		TSharedPtr<FBlueprintEditor> BlueprintEditor = WeakBlueprintEditor.Pin();
		if (BlueprintEditor.IsValid())
		{
			return BlueprintEditor->GetPreviewActor();
		}
		if (UExSequence* Sequence = WeakSequence.Get())
		{
			return Sequence->GetTypedOuter<AActor>();
		}
		return nullptr;
	}

	UActorComponent* GetOwnerComponent() const
	{
		UExSequence* ExSequence = WeakSequence.Get();
		AActor* Actor = ExSequence ? GetPreviewActor() : nullptr;

		return Actor ? FindObject<UActorComponent>(Actor, *ExSequence->GetOuter()->GetName()) : nullptr;
	}

	void OnSequenceChanged()
	{
		UExSequence* ExSequence = WeakSequence.Get();
		UBlueprint* Blueprint = ExSequence ? ExSequence->GetParentBlueprint() : nullptr;

		if (Blueprint)
		{
			FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
		}
	}

private:

	TWeakObjectPtr<UExSequence> WeakSequence;

	TWeakPtr<FBlueprintEditor> WeakBlueprintEditor;

	TSharedPtr<SBox> Content;
	TSharedPtr<ISequencer> Sequencer;

	FDelegateHandle OnBlueprintPreCompileHandle;
	FDelegateHandle OnObjectSavedHandle;

	FDelegateHandle OnSequenceChangedHandle;
};

void SExSequenceEditorWidget::Construct(const FArguments&, TWeakPtr<FBlueprintEditor> InBlueprintEditor)
{
	ChildSlot
	[
		SAssignNew(Impl, SExSequenceEditorWidgetImpl, InBlueprintEditor)
	];
}

FText SExSequenceEditorWidget::GetDisplayLabel() const
{
	return Impl.Pin()->GetDisplayLabel();
}

void SExSequenceEditorWidget::AssignSequence(UExSequence* NewExSequence)
{
	Impl.Pin()->SetExSequence(NewExSequence);
}

UExSequence* SExSequenceEditorWidget::GetSequence() const
{
	return Impl.Pin()->GetExSequence();
}

FExSequenceEditorSummoner::FExSequenceEditorSummoner(TSharedPtr<FBlueprintEditor> BlueprintEditor)
	: FWorkflowTabFactory("EmbeddedExSequenceID", BlueprintEditor)
	, WeakBlueprintEditor(BlueprintEditor)
{
	bIsSingleton = true;

	TabLabel = LOCTEXT("SequencerTabName", "ExSequencer");
	TabIcon = FSlateIcon(FExSequenceEditorStyle::Get().GetStyleSetName(), "ClassIcon.ExSequence");
}

TSharedRef<SWidget> FExSequenceEditorSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return SNew(SExSequenceEditorWidget, WeakBlueprintEditor);
}

#undef LOCTEXT_NAMESPACE
