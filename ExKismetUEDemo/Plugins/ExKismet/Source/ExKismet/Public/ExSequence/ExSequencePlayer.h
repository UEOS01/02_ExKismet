
#pragma once

#include "IMovieScenePlayer.h"
#include "ExSequence.h"
#include "MovieSceneSequencePlayer.h"
#include "ExSequencePlayer.generated.h"

/**
 * UExSequencePlayer is used to actually "play" an sequence asset at runtime.
 */
UCLASS(NotBlueprintType)
class EXKISMET_API UExSequencePlayer
	: public UMovieSceneSequencePlayer
{
public:
	GENERATED_BODY()

protected:

	//~ IMovieScenePlayer interface
	virtual UObject* GetPlaybackContext() const override;
	virtual TArray<UObject*> GetEventContexts() const override;

	/**
	 * Called before destroying the object.  This is called immediately upon deciding to destroy the object, to allow the object to begin an
	 * asynchronous cleanup process.
	 */
	virtual void BeginDestroy() override;
};

