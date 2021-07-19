// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "KismetNodes/SGraphNodeK2Base.h"

class SVerticalBox;
class UK2Node;

class EXKISMETEDITOR_API SExGraphNodeK2Sequence : public SGraphNodeK2Base
{
public:

	SLATE_BEGIN_ARGS(SExGraphNodeK2Sequence) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UK2Node* InNode);

protected:
	// SGraphNode interface
	virtual void CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox) override;
	virtual FReply OnAddPin() override;
	virtual EVisibility IsAddPinButtonVisible() const override;
	/**
     * Update this GraphNode to match the data that it is observing
     */
	virtual void UpdateGraphNode() override;
	// End of SGraphNode interface
};
