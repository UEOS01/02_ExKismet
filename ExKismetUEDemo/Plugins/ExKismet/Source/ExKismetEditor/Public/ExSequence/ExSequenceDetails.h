// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Visibility.h"
#include "Widgets/SWidget.h"
#include "IDetailCustomization.h"
#include "IPropertyTypeCustomization.h"

class IDetailLayoutBuilder;
class IPropertyHandle;
class SComboButton;
class SSearchBox;

DECLARE_LOG_CATEGORY_EXTERN(LogExSequenceDetails, Log, All);

class FExSequenceDetails : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	FExSequenceDetails()
	{
	}

protected:

	virtual void CustomizeHeader( TSharedRef<class IPropertyHandle> PropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils ) override;
	virtual void CustomizeChildren( TSharedRef<class IPropertyHandle> PropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils ) override;

	UExSequence* GetExSequence();

private:

	FReply InvokeExSequencer();

private:

	TSharedPtr<SButton> OpenInTabButton;
	TSharedPtr<IPropertyUtilities> PropertyUtilities;
	TSharedPtr<SBox> InlineSequencer;
	TWeakObjectPtr<UExSequenceAgent> SequenceAgent;
};
