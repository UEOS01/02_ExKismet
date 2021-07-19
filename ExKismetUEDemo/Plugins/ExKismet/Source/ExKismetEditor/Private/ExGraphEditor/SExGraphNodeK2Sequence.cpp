// Copyright Epic Games, Inc. All Rights Reserved.


#include "ExGraphEditor/SExGraphNodeK2Sequence.h"
#include "EdGraph/EdGraph.h"
#include "Widgets/SBoxPanel.h"
#include "GraphEditorSettings.h"
#include "K2Node_AddPinInterface.h"
#include "K2Node.h"
#include "ScopedTransaction.h"
#include "SlateOptMacros.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "SCommentBubble.h"
#include "SGraphPin.h"
#include "GraphEditorDragDropAction.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_Literal.h"
#include "NodeFactory.h"
#include "Logging/TokenizedMessage.h"
#include "DragAndDrop/ActorDragDropGraphEdOp.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "Editor/Persona/Public/BoneDragDropOp.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "SLevelOfDetailBranchNode.h"
#include "Widgets/SToolTip.h"
#include "IDocumentation.h"
#include "TutorialMetaData.h"
#include "SGraphPanel.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "BlueprintEditorSettings.h"


void SExGraphNodeK2Sequence::Construct(const FArguments& InArgs, UK2Node* InNode)
{
	ensure(InNode == nullptr || InNode->GetClass()->ImplementsInterface(UK2Node_AddPinInterface::StaticClass()));
	GraphNode = InNode;

	SetCursor(EMouseCursor::CardinalCross);

	UpdateGraphNode();
}

void SExGraphNodeK2Sequence::CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox)
{
	TSharedRef<SWidget> AddPinButton = AddPinButtonContent(
		NSLOCTEXT("SequencerNode", "SequencerNodeAddPinButton", "Add pin"),
		NSLOCTEXT("SequencerNode", "SequencerNodeAddPinButton_ToolTip", "Add new pin"));

	FMargin AddPinPadding = Settings->GetOutputPinPadding();
	AddPinPadding.Top += 6.0f;

	OutputBox->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Right)
		.Padding(AddPinPadding)
		[
			AddPinButton
		];
}

FReply SExGraphNodeK2Sequence::OnAddPin()
{
	IK2Node_AddPinInterface* AddPinNode = Cast<IK2Node_AddPinInterface>(GraphNode);
	ensure(AddPinNode);
	if (AddPinNode && AddPinNode->CanAddPin())
	{
		FScopedTransaction Transaction(NSLOCTEXT("SequencerNode", "AddPinTransaction", "Add Pin"));

		AddPinNode->AddInputPin();
		UpdateGraphNode();
		GraphNode->GetGraph()->NotifyGraphChanged();
	}

	return FReply::Handled();
}

EVisibility SExGraphNodeK2Sequence::IsAddPinButtonVisible() const
{
	IK2Node_AddPinInterface* AddPinNode = Cast<IK2Node_AddPinInterface>(GraphNode);
	ensure(AddPinNode);
	return ((AddPinNode && AddPinNode->CanAddPin()) ? EVisibility::Visible : EVisibility::Collapsed);
}

/**
 * Update this GraphNode to match the data that it is observing
 */
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SExGraphNodeK2Sequence::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	//
	//             ______________________
	//            |      TITLE AREA      |
	//            +-------+------+-------+
	//            | (>) L |      | R (>) |
	//            | (>) E |      | I (>) |
	//            | (>) F |      | G (>) |
	//            | (>) T |      | H (>) |
	//            |       |      | T (>) |
	//            |_______|______|_______|
	//
	TSharedPtr<SVerticalBox> MainVerticalBox;
	SetupErrorReporting();

	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

	// Get node icon
	IconColor = FLinearColor::White;
	const FSlateBrush* IconBrush = nullptr;
	if (GraphNode != NULL && GraphNode->ShowPaletteIconOnNode())
	{
		IconBrush = GraphNode->GetIconAndTint(IconColor).GetOptionalIcon();
	}

	TSharedRef<SOverlay> DefaultTitleAreaWidget =
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(FEditorStyle::GetBrush("Graph.Node.TitleGloss"))
		.ColorAndOpacity(this, &SGraphNode::GetNodeTitleIconColor)
		]
	+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Graph.Node.ColorSpill"))
		// The extra margin on the right
		// is for making the color spill stretch well past the node title
		.Padding(FMargin(10, 5, 30, 3))
		.BorderBackgroundColor(this, &SGraphNode::GetNodeTitleColor)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.VAlign(VAlign_Top)
		.Padding(FMargin(0.f, 0.f, 4.f, 0.f))
		.AutoWidth()
		[
			SNew(SImage)
			.Image(IconBrush)
		.ColorAndOpacity(this, &SGraphNode::GetNodeTitleIconColor)
		]
	+ SHorizontalBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateTitleWidget(NodeTitle)
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			NodeTitle.ToSharedRef()
		]
		]
		]
		]
	+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(0, 0, 5, 0)
		.AutoWidth()
		[
			CreateTitleRightWidget()
		]
		]
	+ SOverlay::Slot()
		.VAlign(VAlign_Top)
		[
			SNew(SBorder)
			.Visibility(EVisibility::HitTestInvisible)
		.BorderImage(FEditorStyle::GetBrush("Graph.Node.TitleHighlight"))
		.BorderBackgroundColor(this, &SGraphNode::GetNodeTitleIconColor)
		[
			SNew(SSpacer)
			.Size(FVector2D(20, 20))
		]
		];

	SetDefaultTitleAreaWidget(DefaultTitleAreaWidget);

	TSharedRef<SWidget> TitleAreaWidget =
		SNew(SLevelOfDetailBranchNode)
		.UseLowDetailSlot(this, &SExGraphNodeK2Sequence::UseLowDetailNodeTitles)
		.LowDetail()
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Graph.Node.ColorSpill"))
		.Padding(FMargin(75.0f, 22.0f)) // Saving enough space for a 'typical' title so the transition isn't quite so abrupt
		.BorderBackgroundColor(this, &SGraphNode::GetNodeTitleColor)
		]
	.HighDetail()
		[
			DefaultTitleAreaWidget
		];


	if (!SWidget::GetToolTip().IsValid())
	{
		TSharedRef<SToolTip> DefaultToolTip = IDocumentation::Get()->CreateToolTip(TAttribute< FText >(this, &SGraphNode::GetNodeTooltip), NULL, GraphNode->GetDocumentationLink(), GraphNode->GetDocumentationExcerptName());
		SetToolTip(DefaultToolTip);
	}

	// Setup a meta tag for this node
	FGraphNodeMetaData TagMeta(TEXT("Graphnode"));
	PopulateMetaTag(&TagMeta);

	TSharedPtr<SVerticalBox> InnerVerticalBox;
	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);


	InnerVerticalBox = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		.Padding(Settings->GetNonPinNodeBodyPadding())
		[
			TitleAreaWidget
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		[
			CreateNodeContentArea()
		];

	TSharedPtr<SWidget> EnabledStateWidget = GetEnabledStateWidget();
	if (EnabledStateWidget.IsValid())
	{
		InnerVerticalBox->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			.Padding(FMargin(2, 0))
			[
				EnabledStateWidget.ToSharedRef()
			];
	}

	InnerVerticalBox->AddSlot()
		.AutoHeight()
		.Padding(Settings->GetNonPinNodeBodyPadding())
		[
			ErrorReporting->AsWidget()
		];



	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(MainVerticalBox, SVerticalBox)
			+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SOverlay)
			.AddMetaData<FGraphNodeMetaData>(TagMeta)
		+ SOverlay::Slot()
		.Padding(Settings->GetNonPinNodeBodyPadding())
		[
			SNew(SImage)
			.Image(GetNodeBodyBrush())
		.ColorAndOpacity(this, &SGraphNode::GetNodeBodyColor)
		]
	+ SOverlay::Slot()
		[
			InnerVerticalBox.ToSharedRef()
		]
		]
		];

	bool SupportsBubble = true;
	if (GraphNode != nullptr)
	{
		SupportsBubble = GraphNode->SupportsCommentBubble();
	}

	if (SupportsBubble)
	{
		// Create comment bubble
		TSharedPtr<SCommentBubble> CommentBubble;
		const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

		SAssignNew(CommentBubble, SCommentBubble)
			.GraphNode(GraphNode)
			.Text(this, &SGraphNode::GetNodeComment)
			.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
			.OnToggled(this, &SGraphNode::OnCommentBubbleToggled)
			.ColorAndOpacity(CommentColor)
			.AllowPinning(true)
			.EnableTitleBarBubble(true)
			.EnableBubbleCtrls(true)
			.GraphLOD(this, &SGraphNode::GetCurrentLOD)
			.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

		GetOrAddSlot(ENodeZone::TopCenter)
			.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
			.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
			.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
			.VAlign(VAlign_Top)
			[
				CommentBubble.ToSharedRef()
			];
	}

	CreateBelowWidgetControls(MainVerticalBox);
	CreatePinWidgets();
	CreateInputSideAddButton(LeftNodeBox);
	CreateOutputSideAddButton(RightNodeBox);
	CreateBelowPinControls(InnerVerticalBox);
	CreateAdvancedViewArrow(InnerVerticalBox);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION