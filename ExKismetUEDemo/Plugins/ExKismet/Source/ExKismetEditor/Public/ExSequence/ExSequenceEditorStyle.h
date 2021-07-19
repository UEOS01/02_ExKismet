
#pragma once

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"

class FExSequenceEditorStyle
	: public FSlateStyleSet
{
public:
	FExSequenceEditorStyle()
		: FSlateStyleSet("ExSequenceEditorStyle")
	{
		const FVector2D Icon16x16(16.0f, 16.0f);

		SetContentRoot(IPluginManager::Get().FindPlugin("ExKismet")->GetBaseDir() / TEXT("Resources/ExSequence"));

		Set("ClassIcon.ExSequence", new FSlateImageBrush(RootToContentDir(TEXT("ExSequence_16x.png")), Icon16x16));
		Set("ClassIcon.ExSequenceAgent", new FSlateImageBrush(RootToContentDir(TEXT("ExSequence_16x.png")), Icon16x16));

		FSlateStyleRegistry::RegisterSlateStyle(*this);
	}

	static FExSequenceEditorStyle& Get()
	{
		static FExSequenceEditorStyle Inst;
		return Inst;
	}
	
	~FExSequenceEditorStyle()
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	}
};
