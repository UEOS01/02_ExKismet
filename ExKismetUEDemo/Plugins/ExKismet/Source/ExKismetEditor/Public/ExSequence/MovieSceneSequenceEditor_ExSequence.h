
#pragma once

#include "MovieSceneSequenceEditor.h"
#include "ExSequence/ExSequence.h"
#include "Engine/Level.h"
#include "Engine/LevelScriptBlueprint.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Tracks/MovieSceneEventTrack.h"
#include "K2Node_FunctionEntry.h"
#include "EdGraphSchema_K2.h"

struct FMovieSceneSequenceEditor_ExSequence : FMovieSceneSequenceEditor
{
	virtual bool CanCreateEvents(UMovieSceneSequence* InSequence) const
	{
		return true;
	}

	virtual UBlueprint* GetBlueprintForSequence(UMovieSceneSequence* InSequence) const override
	{
		UExSequence* ExSequence = CastChecked<UExSequence>(InSequence);
		if (UBlueprint* Blueprint = ExSequence->GetParentBlueprint())
		{
			return Blueprint;
		}

		UExSequenceAgent* Agent = ExSequence->GetTypedOuter<UExSequenceAgent>();
		ULevel* Level = Agent ? Agent->GetTypedOuter<ULevel>() : nullptr;

		bool bDontCreateNewBlueprint = true;
		return Level ? Level->GetLevelScriptBlueprint(bDontCreateNewBlueprint) : nullptr;
	}

	virtual UBlueprint* CreateBlueprintForSequence(UMovieSceneSequence* InSequence) const override
	{
		UExSequence* ExSequence = CastChecked<UExSequence>(InSequence);
		check(!ExSequence->GetParentBlueprint());

		UExSequenceAgent* Agent = ExSequence->GetTypedOuter<UExSequenceAgent>();
		ULevel* Level = Agent ? Agent->GetTypedOuter<ULevel>() : nullptr;

		bool bDontCreateNewBlueprint = false;
		return Level ? Level->GetLevelScriptBlueprint(bDontCreateNewBlueprint) : nullptr;
	}
};