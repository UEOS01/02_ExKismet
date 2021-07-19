
#pragma once

#include "MovieSceneSequencePlayer.h"
#include "Tickable.h"
#include "ExSequenceAgent.generated.h"


class UExSequence;
class UExSequencePlayer;


/**
 * Sequence Object Agent embedded within any other UObject.
 */
UCLASS(Blueprintable, BlueprintType, editinlinenew, DefaultToInstanced, ClassGroup=(ExKismet,ExSequence), hidecategories=(Cooking))
class EXKISMET_API UExSequenceAgent
	: public UObject, public FTickableGameObject
{
public:

	GENERATED_BODY()

	UExSequenceAgent(const FObjectInitializer& Init);

	virtual UWorld* GetWorld() const override;

	// FTickableGameObject begin
	virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorldFast(); }
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	// FTickableGameObject end

	UWorld* GetWorldFast() const { return GetOuter()->GetWorld(); }

	UFUNCTION(BlueprintPure)
	UExSequence* GetSequence() const
	{
		return Sequence;
	}

	UFUNCTION(BlueprintPure)
	UExSequencePlayer* GetSequencePlayer();

	UFUNCTION(BlueprintCallable)
	void Play();
	
	virtual void PostInitProperties() override;
	/**
	 * Called before destroying the object.  This is called immediately upon deciding to destroy the object, to allow the object to begin an
	 * asynchronous cleanup process.
	 */
	virtual void BeginDestroy() override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Playback", meta=(ShowOnlyInnerProperties))
	FMovieSceneSequencePlaybackSettings PlaybackSettings;

	/** Embedded sequence data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category=Animation)
	UExSequence* Sequence;

private:

	UPROPERTY()
	bool bIsFirstTick = true;

	UPROPERTY(transient)
	UExSequencePlayer* SequencePlayer;
};
