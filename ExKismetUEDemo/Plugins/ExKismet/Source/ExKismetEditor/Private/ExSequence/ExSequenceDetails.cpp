// Copyright Epic Games, Inc. All Rights Reserved.

#include "ExSequence/ExSequenceDetails.h"
#include "UObject/UnrealType.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSlider.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "UObject/UObjectHash.h"
#include "UObject/UObjectIterator.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Views/SListView.h"
#include "SListViewSelectorDropdownMenu.h"
#include "Widgets/Input/STextComboBox.h"
#include "IDetailsView.h"

#define LOCTEXT_NAMESPACE "ExSequenceDetails"

DEFINE_LOG_CATEGORY(LogExSequenceDetails);

TSharedRef<IPropertyTypeCustomization> FExSequenceDetails::MakeInstance()
{
	return MakeShareable(new FExSequenceDetails());
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void FExSequenceDetails::CustomizeHeader( TSharedRef<class IPropertyHandle> PropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils )
{
	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);
	
	UObject* OuterOfProperty = nullptr;
	if (OuterObjects.Num() > 0)
	{
		OuterOfProperty = OuterObjects[0];
	}

	if (OuterOfProperty)
	{
		SequenceAgent = Cast<UExSequenceAgent>(OuterOfProperty);
	}

	bool bIsExternalExSequencerTabAlreadyOpened = false;

	PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

	HeaderRow
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(600)
	.MaxDesiredWidth(4096)
	[
		// PropertyHandle->CreatePropertyValueWidget()
		SAssignNew(OpenInTabButton,SButton)
		.OnClicked(this, &FExSequenceDetails::InvokeExSequencer)
		[
			SNew(STextBlock)
			.Text(bIsExternalExSequencerTabAlreadyOpened ? LOCTEXT("FocusExSequenceTabButtonText", "Focus Tab") : LOCTEXT("OpenExSequenceTabButtonText", "Open in Tab"))
		]
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION


//BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
//void FExSequenceDetails::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
//{
//	uint32 NumChildren = 0;
//	StructPropertyHandle->GetNumChildren(NumChildren);
//
//	ValueProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FScalableFloat, Value));
//	CurveTableHandleProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FScalableFloat, Curve));
//
//	if (ValueProperty.IsValid() && CurveTableHandleProperty.IsValid())
//	{
//		RowNameProperty = CurveTableHandleProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FCurveTableRowHandle, RowName));
//		CurveTableProperty = CurveTableHandleProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FCurveTableRowHandle, CurveTable));
//
//		CurrentSelectedItem = InitWidgetContent();
//
//		FSimpleDelegate OnCurveTableChangedDelegate = FSimpleDelegate::CreateSP(this, &FExSequenceDetails::OnCurveTableChanged);
//		CurveTableProperty->SetOnPropertyValueChanged(OnCurveTableChangedDelegate);
//
//		HeaderRow
//			.NameContent()
//			[
//				StructPropertyHandle->CreatePropertyNameWidget()
//			]
//		.ValueContent()
//			.MinDesiredWidth(600)
//			.MaxDesiredWidth(4096)
//			[
//				SNew(SHorizontalBox)
//				.IsEnabled(TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateSP(this, &FExSequenceDetails::IsEditable)))
//
//			+ SHorizontalBox::Slot()
//			.FillWidth(0.12f)
//			.HAlign(HAlign_Fill)
//			.Padding(0.f, 0.f, 2.f, 0.f)
//			[
//				ValueProperty->CreatePropertyValueWidget()
//			]
//
//		+ SHorizontalBox::Slot()
//			.FillWidth(0.40f)
//			.HAlign(HAlign_Fill)
//			.Padding(2.f, 0.f, 2.f, 0.f)
//			[
//				CreateCurveTableWidget()
//			]
//
//		+ SHorizontalBox::Slot()
//			.FillWidth(0.23f)
//			.HAlign(HAlign_Fill)
//			.VAlign(VAlign_Center)
//			.Padding(2.f, 0.f, 2.f, 0.f)
//			[
//				SAssignNew(RowNameComboButton, SComboButton)
//				.OnGetMenuContent(this, &FExSequenceDetails::GetListContent)
//			.ContentPadding(FMargin(2.0f, 2.0f))
//			.Visibility(this, &FExSequenceDetails::GetRowNameVisibility)
//			.ButtonContent()
//			[
//				SNew(STextBlock)
//				.Text(this, &FExSequenceDetails::GetRowNameComboBoxContentText)
//			.ToolTipText(this, &FExSequenceDetails::GetRowNameComboBoxContentText)
//			]
//			]
//
//		+ SHorizontalBox::Slot()
//			.FillWidth(0.15f)
//			.HAlign(HAlign_Fill)
//			.Padding(2.f, 0.f, 2.f, 0.f)
//			[
//				SNew(SVerticalBox)
//				.Visibility(this, &FExSequenceDetails::GetPreviewVisibility)
//
//			+ SVerticalBox::Slot()
//			.HAlign(HAlign_Center)
//			[
//				SNew(STextBlock)
//				.Text(this, &FExSequenceDetails::GetRowValuePreviewLabel)
//			]
//
//		+ SVerticalBox::Slot()
//			.HAlign(HAlign_Center)
//			[
//				SNew(STextBlock)
//				.Text(this, &FExSequenceDetails::GetRowValuePreviewText)
//			]
//			]
//
//		+ SHorizontalBox::Slot()
//			.FillWidth(0.1f)
//			.HAlign(HAlign_Fill)
//			.Padding(2.f, 0.f, 0.f, 0.f)
//			[
//				SNew(SSlider)
//				.Visibility(this, &FExSequenceDetails::GetPreviewVisibility)
//			.ToolTipText(LOCTEXT("LevelPreviewToolTip", "Adjust the preview level."))
//			.Value(this, &FExSequenceDetails::GetPreviewLevel)
//			.OnValueChanged(this, &FExSequenceDetails::SetPreviewLevel)
//			]
//			];
//	}
//}
//
//END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply FExSequenceDetails::InvokeExSequencer()
{
	TSharedPtr<FTabManager> TabManager;

	FWidgetPath WidgetPath;
	if (FSlateApplication::Get().FindPathToWidget(OpenInTabButton.ToSharedRef(), WidgetPath, EVisibility::All))
	{
		const int32 WidgetIndex = WidgetPath.Widgets.FindLastByPredicate([](const FArrangedWidget& SomeWidget)
			{
				return SomeWidget.Widget->GetTypeAsString() == FString(TEXT("SDetailsView"));
			});
		check(WidgetIndex != INDEX_NONE);
		TSharedRef<IDetailsView> FirstDetailsView = StaticCastSharedRef<IDetailsView>(WidgetPath.Widgets[WidgetIndex].Widget);
		TabManager = FirstDetailsView->GetHostTabManager();
	}

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

			Tab->SetLabel(TAttribute<FText>(LOCTEXT("ExSequencerTabLabel", "ExSequencer")));
			// Tab->SetTabLabelSuffix(TAttribute<FText>(LOCTEXT("ExSequencerTabLabelSuffix", "ExSequencer")));
		}
	}

	PropertyUtilities->ForceRefresh();

	return FReply::Handled();
}

void FExSequenceDetails::CustomizeChildren(TSharedRef<class IPropertyHandle> PropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{

}

UExSequence* FExSequenceDetails::GetExSequence()
{
	if (SequenceAgent.IsValid())
	{
		return SequenceAgent->GetSequence();
	}
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
