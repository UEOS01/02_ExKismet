
#pragma once

#include "WorkflowOrientedApp/WorkflowUObjectDocuments.h"


class FBlueprintEditor;
class SExSequenceEditorWidgetImpl;
class UExSequence;
class UExSequenceAgent;

class SExSequenceEditorWidget
	: public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SExSequenceEditorWidget){}
	SLATE_END_ARGS();

	void Construct(const FArguments&, TWeakPtr<FBlueprintEditor> InBlueprintEditor);
	void AssignSequence(UExSequence* NewExSequence);
	UExSequence* GetSequence() const;
	FText GetDisplayLabel() const;

private:

	TWeakPtr<SExSequenceEditorWidgetImpl> Impl;
};


struct FExSequenceEditorSummoner
	: public FWorkflowTabFactory
{
	FExSequenceEditorSummoner(TSharedPtr<FBlueprintEditor> BlueprintEditor);

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;

protected:

	TWeakObjectPtr<UExSequenceAgent> WeakAgent;
	TWeakPtr<FBlueprintEditor> WeakBlueprintEditor;
};
