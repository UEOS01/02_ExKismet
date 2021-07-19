
#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Widgets/Layout/SBox.h"
#include "Input/Reply.h"
#include "UObject/WeakObjectPtr.h"
#include "Framework/Docking/TabManager.h"

class IDetailsView;
class UExSequence;
class UExSequenceAgent;
class ISequencer;
class FSCSEditorTreeNode;
class IPropertyUtilities;

class FExSequenceAgentCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:

	FReply InvokeSequencer();
	UExSequence* GetExSequence() const;

	TWeakObjectPtr<UExSequenceAgent> WeakSequenceAgent;
	TWeakPtr<FTabManager> WeakTabManager;
	TSharedPtr<SBox> InlineSequencer;
	TSharedPtr<IPropertyUtilities> PropertyUtilities;
};