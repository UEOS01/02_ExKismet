
#include "ExSequence/ExSequencePlayer.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SimpleConstructionScript.h"

UObject* UExSequencePlayer::GetPlaybackContext() const
{
	UExSequence* ExSequence = CastChecked<UExSequence>(Sequence);

	if (ExSequence)
	{
		UObject* OuterContext = ExSequence->GetOuter();
		if (OuterContext && OuterContext->GetOuter())
		{
			return OuterContext->GetOuter();
		}
		else if (AActor* Actor = ExSequence->GetTypedOuter<AActor>())
		{
			return Actor;
		}
#if WITH_EDITOR
		else if (UBlueprintGeneratedClass* GeneratedClass = ExSequence->GetTypedOuter<UBlueprintGeneratedClass>())
		{
			return GeneratedClass->SimpleConstructionScript->GetComponentEditorActorInstance();
		}
#endif
	}

	return nullptr;
}

TArray<UObject*> UExSequencePlayer::GetEventContexts() const
{
	TArray<UObject*> Contexts;
	if (UObject* PlaybackContext = GetPlaybackContext())
	{
		Contexts.Add(PlaybackContext);
	}
	return Contexts;
}

void UExSequencePlayer::BeginDestroy()
{
	Stop();
	Super::BeginDestroy();
}