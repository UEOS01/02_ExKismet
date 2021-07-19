
#include "ExSequence/ExSequenceAgentCustomization.h"

#include "ExSequence/ExSequence.h"
#include "ExSequence/ExSequenceAgent.h"
#include "EditorStyleSet.h"
#include "GameFramework/Actor.h"
#include "IDetailsView.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Docking/SDockTab.h"
#include "SSCSEditor.h"
#include "BlueprintEditorTabs.h"
#include "ScopedTransaction.h"
#include "ISequencerModule.h"
#include "Editor.h"
#include "ExSequence/ExSequenceEditorTabSummoner.h"
#include "IPropertyUtilities.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "ExSequenceAgentCustomization"

FName SequenceTabId("EmbeddedExSequenceID");

class SExSequenceEditorWidgetWrapper : public SExSequenceEditorWidget
{
public:
	~SExSequenceEditorWidgetWrapper()
	{
		GEditor->OnObjectsReplaced().Remove(OnObjectsReplacedHandle);
	}

	void Construct(const FArguments& InArgs, TWeakObjectPtr<UExSequenceAgent> InSequenceAgent)
	{
		SExSequenceEditorWidget::Construct(InArgs, nullptr);

		WeakSequenceAgent = InSequenceAgent;
		AssignSequence(GetExSequence());

		OnObjectsReplacedHandle = GEditor->OnObjectsReplaced().AddSP(this, &SExSequenceEditorWidgetWrapper::OnObjectsReplaced);
	}

protected:

	UExSequence* GetExSequence() const
	{
		UExSequenceAgent* SequenceAgent = WeakSequenceAgent.Get();
		return SequenceAgent ? SequenceAgent->GetSequence() : nullptr;
	}

	void OnObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap)
	{
		UExSequenceAgent* Agent = WeakSequenceAgent.Get(true);

		UExSequenceAgent* NewSequenceAgent = Agent ? Cast<UExSequenceAgent>(ReplacementMap.FindRef(Agent)) : nullptr;
		if (NewSequenceAgent)
		{
			WeakSequenceAgent = NewSequenceAgent;
			AssignSequence(GetExSequence());
		}
	}

private:

	TWeakObjectPtr<UExSequenceAgent> WeakSequenceAgent;
	FDelegateHandle OnObjectsReplacedHandle;
};

TSharedRef<IDetailCustomization> FExSequenceAgentCustomization::MakeInstance()
{
	return MakeShared<FExSequenceAgentCustomization>();
}

void FExSequenceAgentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	PropertyUtilities = DetailBuilder.GetPropertyUtilities();

	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() != 1)
	{
		// @todo: Display other common properties / information?
		return;
	}

	WeakSequenceAgent = Cast<UExSequenceAgent>(Objects[0].Get());
	if (!WeakSequenceAgent.Get())
	{
		return;
	}

	const IDetailsView* DetailsView = DetailBuilder.GetDetailsView();
	TSharedPtr<FTabManager> HostTabManager = DetailsView->GetHostTabManager();

	DetailBuilder.HideProperty("Sequence");

	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Sequence", FText(), ECategoryPriority::Important);

	bool bIsExternalTabAlreadyOpened = false;

	if (HostTabManager.IsValid() && HostTabManager->HasTabSpawner(SequenceTabId))
	{
		WeakTabManager = HostTabManager;

		TSharedPtr<SDockTab> ExistingTab = HostTabManager->FindExistingLiveTab(SequenceTabId);
		if (ExistingTab.IsValid())
		{
			UExSequence* ThisSequence = GetExSequence();

			auto SequencerWidget = StaticCastSharedRef<SExSequenceEditorWidget>(ExistingTab->GetContent());
			bIsExternalTabAlreadyOpened = ThisSequence && SequencerWidget->GetSequence() == ThisSequence;
		}

		Category.AddCustomRow(FText())
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SequenceValueText", "Sequence"))
				.Font(DetailBuilder.GetDetailFont())
			]
			.ValueContent()
			[
				SNew(SButton)
				.OnClicked(this, &FExSequenceAgentCustomization::InvokeSequencer)
				[
					SNew(STextBlock)
					.Text(bIsExternalTabAlreadyOpened ? LOCTEXT("FocusSequenceTabButtonText", "Focus Tab") : LOCTEXT("OpenSequenceTabButtonText", "Open in Tab"))
					.Font(DetailBuilder.GetDetailFont())
				]
			];
	}

	// Only display an inline editor for non-blueprint sequences
	if (GetExSequence() && !GetExSequence()->GetParentBlueprint() && !bIsExternalTabAlreadyOpened)
	{
		Category.AddCustomRow(FText())
		.WholeRowContent()
		.MaxDesiredWidth(TOptional<float>())
		[
			SAssignNew(InlineSequencer, SBox)
			.HeightOverride(300)
			[
				SNew(SExSequenceEditorWidgetWrapper, WeakSequenceAgent)
			]
		];
	}
}

FReply FExSequenceAgentCustomization::InvokeSequencer()
{
	TSharedPtr<FTabManager> TabManager = WeakTabManager.Pin();
	if (TabManager.IsValid() && TabManager->HasTabSpawner(SequenceTabId))
	{
		if (TSharedPtr<SDockTab> Tab = TabManager->TryInvokeTab(SequenceTabId))
		{
			{
				// Set up a delegate that forces a refresh of this panel when the tab is closed to ensure we see the inline widget
				TWeakPtr<IPropertyUtilities> WeakUtilities = PropertyUtilities;
				auto OnClosed = [WeakUtilities](TSharedRef<SDockTab>)
				{
					TSharedPtr<IPropertyUtilities> PinnedPropertyUtilities = WeakUtilities.Pin();
					if (PinnedPropertyUtilities.IsValid())
					{
						PinnedPropertyUtilities->EnqueueDeferredAction(FSimpleDelegate::CreateSP(PinnedPropertyUtilities.ToSharedRef(), &IPropertyUtilities::ForceRefresh));
					}
				};

				Tab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateLambda(OnClosed));
			}

			// Move our inline widget content to the tab (so that we keep the existing sequencer state)
			if (InlineSequencer.IsValid())
			{
				Tab->SetContent(InlineSequencer->GetChildren()->GetChildAt(0));
				InlineSequencer->SetContent(SNullWidget::NullWidget);
				InlineSequencer->SetVisibility(EVisibility::Collapsed);
			}
			else
			{
				StaticCastSharedRef<SExSequenceEditorWidget>(Tab->GetContent())->AssignSequence(GetExSequence());
			}
		}
	}

	PropertyUtilities->ForceRefresh();

	return FReply::Handled();
}

UExSequence* FExSequenceAgentCustomization::GetExSequence() const
{
	UExSequenceAgent* SequenceAgent = WeakSequenceAgent.Get();
	return SequenceAgent ? SequenceAgent->GetSequence() : nullptr;
}

#undef LOCTEXT_NAMESPACE
