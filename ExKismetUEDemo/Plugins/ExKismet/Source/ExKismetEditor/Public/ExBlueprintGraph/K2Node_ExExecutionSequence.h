// Copyright Epic Games, Inc. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Textures/SlateIcon.h"
#include "K2Node.h"
#include "K2Node_AddPinInterface.h"
#include "EdGraph/EdGraphPin.h"

#include "K2Node_ExExecutionSequence.generated.h"

class FBlueprintActionDatabaseRegistrar;

UCLASS(MinimalAPI, meta = (Keywords = "ExSequence"))
class UK2Node_ExExecutionSequence : public UK2Node, public IK2Node_AddPinInterface
{
	GENERATED_UCLASS_BODY()

	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetTooltipText() const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	/** Create a visual widget to represent this node in a graph editor or graph panel.  If not implemented, the default node factory will be used. */
	virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;
	//~ End UEdGraphNode Interface

	//~ Begin UK2Node Interface
	void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	virtual class FNodeHandlingFunctor* CreateNodeHandler(class FKismetCompilerContext& CompilerContext) const override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual bool CanEverInsertExecutionPin() const override { return true; }
	virtual bool CanEverRemoveExecutionPin() const override { return true; }
	//~ End UK2Node Interface

	// IK2Node_AddPinInterface interface
	virtual void AddInputPin() override;
	// End of IK2Node_AddPinInterface interface

	//~ Begin K2Node_ExExecutionSequence Interface

	/** Gets a unique pin name, the next in the sequence */
	FName GetUniquePinName();

	/**
	 * Inserts a new execution pin, before the specified execution pin, into an execution node
	 *
	 * @param	PinToInsertBefore	The pin to insert a new pin before on the node
	 */
	EXKISMETEDITOR_API void InsertPinIntoExecutionNode(UEdGraphPin* PinToInsertBefore, EPinInsertPosition Position);

	/**
	 * Removes the specified execution pin from an execution node
	 *
	 * @param	TargetPin	The pin to remove from the node
	 */
	EXKISMETEDITOR_API void RemovePinFromExecutionNode(UEdGraphPin* TargetPin);

	/** Whether an execution pin can be removed from the node or not */
	EXKISMETEDITOR_API bool CanRemoveExecutionPin() const;

	// @todo document
	EXKISMETEDITOR_API UEdGraphPin* GetThenPinGivenIndex(int32 Index);

private:
	// Returns the exec output pin name for a given 0-based index
	virtual FName GetPinNameGivenIndex(int32 Index) const;
};

