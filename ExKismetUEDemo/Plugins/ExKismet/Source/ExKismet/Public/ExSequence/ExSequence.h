// Copyright YeHaike. All Rights Reserved.

#pragma once

#include "MovieSceneSequence.h"
#include "MovieScene.h"
#include "ExSequenceObjectReference.h"
#include "ExSequence.generated.h"

/**
 * Sequence embedded within an Blueprint.
 */
UCLASS(NotBlueprintType, DefaultToInstanced)
class EXKISMET_API UExSequence
	: public UMovieSceneSequence
{
public:
	GENERATED_BODY()

	UExSequence(const FObjectInitializer& ObjectInitializer);

	//~ UMovieSceneSequence interface
	virtual void BindPossessableObject(const FGuid& ObjectId, UObject& PossessedObject, UObject* Context) override;
	virtual bool CanPossessObject(UObject& Object, UObject* InPlaybackContext) const override;
	virtual void LocateBoundObjects(const FGuid& ObjectId, UObject* Context, TArray<UObject*, TInlineAllocator<1>>& OutObjects) const override;
	virtual UMovieScene* GetMovieScene() const override;
	virtual UObject* GetParentObject(UObject* Object) const override;
	virtual void UnbindPossessableObjects(const FGuid& ObjectId) override;
	virtual UObject* CreateDirectorInstance(IMovieScenePlayer& Player, FMovieSceneSequenceID SequenceID) override;

#if WITH_EDITOR
	virtual FText GetDisplayName() const override;
	virtual ETrackSupport IsTrackSupported(TSubclassOf<class UMovieSceneTrack> InTrackClass) const override;
#endif

	UBlueprint* GetParentBlueprint() const;

	bool IsEditable() const;
	
private:

	//~ UObject interface
	virtual void PostInitProperties() override;

private:
	
	/** Pointer to the movie scene that controls this animation. */
	UPROPERTY(Instanced)
	UMovieScene* MovieScene;

	/** Collection of object references. */
	UPROPERTY()
	FExSequenceObjectReferenceMap ObjectReferences;

#if WITH_EDITOR
public:

	/** Event that is fired to initialize default state for a sequence */
	DECLARE_EVENT_OneParam(UExSequence, FOnInitialize, UExSequence*)
	static FOnInitialize& OnInitializeSequence() { return OnInitializeSequenceEvent; }

private:
	static FOnInitialize OnInitializeSequenceEvent;
#endif

#if WITH_EDITORONLY_DATA
private:
	UPROPERTY()
	bool bHasBeenInitialized;
#endif
};