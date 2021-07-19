
#include "ExSequence/ExSequenceAgent.h"
#include "ExSequence/ExSequence.h"
#include "ExSequence/ExSequencePlayer.h"

UExSequenceAgent::UExSequenceAgent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject) || GetArchetype() == GetDefault<UExSequenceAgent>())
	{
		Sequence = ObjectInitializer.CreateDefaultSubobject<UExSequence>(this, "Sequence");
		Sequence->SetFlags(RF_Public | RF_Transactional);
	}
}

void UExSequenceAgent::PostInitProperties()
{
	Super::PostInitProperties();
}

void UExSequenceAgent::BeginDestroy()
{
	//if (SequencePlayer)
	//{
	//	// Stop the internal sequence player during EndPlay when it's safer
	//	// to modify other actor's state during state restoration.
	//	SequencePlayer->Stop();
	//}

	Super::BeginDestroy();
}

void UExSequenceAgent::Tick(float DeltaTime)
{
	//if (bIsFirstTick)
	//{
	//	bIsFirstTick = false;
	//	SequencePlayer = GetSequencePlayer();
	//	if (PlaybackSettings.bAutoPlay)
	//	{
	//		SequencePlayer->Play();
	//	}
	//}

	if (SequencePlayer)
	{
		SequencePlayer->Update(DeltaTime);
	}
}

UExSequencePlayer* UExSequenceAgent::GetSequencePlayer()
{
	if (Sequence != nullptr)
	{
		SequencePlayer = NewObject<UExSequencePlayer>(this, "SequencePlayer");
		SequencePlayer->Initialize(Sequence, PlaybackSettings);
	}
	return SequencePlayer;
}

void UExSequenceAgent::Play()
{
	GetSequencePlayer()->Play();
}

UWorld* UExSequenceAgent::GetWorld() const
{
	return GetWorldFast();
}

ETickableTickType UExSequenceAgent::GetTickableTickType() const
{
	// return (HasAnyFlags(RF_ClassDefaultObject) || Sequence == nullptr || SequencePlayer == nullptr)
	//	? ETickableTickType::Never
	//	: ETickableTickType::Always;
	return ETickableTickType::Conditional;
}

bool UExSequenceAgent::IsTickable() const
{
	return Sequence != nullptr && SequencePlayer != nullptr && SequencePlayer->IsPlaying();
}

TStatId UExSequenceAgent::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UExSequenceAgent, STATGROUP_Tickables);
}