
#include "ExSequence/ExSequence.h"
#include "MovieScene.h"
#include "MovieSceneCommonHelpers.h"
#include "Modules/ModuleManager.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "ExSequence/ExSequenceAgent.h"
#include "Engine/LevelScriptActor.h"
#include "Tracks/MovieSceneAudioTrack.h"
//#include "Tracks/MovieSceneCameraCutTrack.h"
//#include "Tracks/MovieSceneCinematicShotTrack.h"
#include "Tracks/MovieSceneEventTrack.h"
#include "Tracks/MovieSceneFadeTrack.h"
#include "Tracks/MovieSceneLevelVisibilityTrack.h"
#include "Tracks/MovieSceneMaterialParameterCollectionTrack.h"
#include "Tracks/MovieSceneSlomoTrack.h"
#include "Tracks/MovieSceneSpawnTrack.h"
#include "Tracks/MovieSceneSubTrack.h"

#if WITH_EDITOR
UExSequence::FOnInitialize UExSequence::OnInitializeSequenceEvent;
#endif

static TAutoConsoleVariable<int32> CVarDefaultEvaluationType(
	TEXT("ExSequence.DefaultEvaluationType"),
	0,
	TEXT("0: Playback locked to playback frames\n1: Unlocked playback with sub frame interpolation"),
	ECVF_Default);

static TAutoConsoleVariable<FString> CVarDefaultTickResolution(
	TEXT("ExSequence.DefaultTickResolution"),
	TEXT("24000fps"),
	TEXT("Specifies default a tick resolution for newly created level sequences. Examples: 30 fps, 120/1 (120 fps), 30000/1001 (29.97), 0.01s (10ms)."),
	ECVF_Default);

static TAutoConsoleVariable<FString> CVarDefaultDisplayRate(
	TEXT("ExSequence.DefaultDisplayRate"),
	TEXT("30fps"),
	TEXT("Specifies default a display frame rate for newly created level sequences; also defines frame locked frame rate where sequences are set to be frame locked. Examples: 30 fps, 120/1 (120 fps), 30000/1001 (29.97), 0.01s (10ms)."),
	ECVF_Default);

UExSequence::UExSequence(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MovieScene(nullptr)
#if WITH_EDITORONLY_DATA
	, bHasBeenInitialized(false)
#endif
{
	bParentContextsAreSignificant = true;

	MovieScene = ObjectInitializer.CreateDefaultSubobject<UMovieScene>(this, "MovieScene");
	MovieScene->SetFlags(RF_Transactional);
}

bool UExSequence::IsEditable() const
{
	UObject* Template = GetArchetype();

	if (Template == GetDefault<UExSequence>())
	{
		return false;
	}

	return !Template || Template->GetTypedOuter<UExSequenceAgent>() == GetDefault<UExSequenceAgent>();
}

UBlueprint* UExSequence::GetParentBlueprint() const
{
	if (UBlueprintGeneratedClass* GeneratedClass = GetTypedOuter<UBlueprintGeneratedClass>())
	{
		return Cast<UBlueprint>(GeneratedClass->ClassGeneratedBy);
	}

	UExSequenceAgent* ExSequenceAgent = GetTypedOuter<UExSequenceAgent>();

	if (ExSequenceAgent)
	{
		UObject* Outer = ExSequenceAgent->GetOuter();
		if (Outer && Outer->StaticClass())
		{
			UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Outer->GetClass());
			if (GeneratedClass)
			{
				return Cast<UBlueprint>(GeneratedClass->ClassGeneratedBy);
			}
		}
	}

	return nullptr;
}

void UExSequence::PostInitProperties()
{
#if WITH_EDITOR && WITH_EDITORONLY_DATA

	// We do not run the default initialization for actor sequences that are CDOs, or that are going to be loaded (since they will have already been initialized in that case)
	EObjectFlags ExcludeFlags = RF_ClassDefaultObject | RF_NeedLoad | RF_NeedPostLoad | RF_NeedPostLoadSubobjects | RF_WasLoaded;

	UExSequenceAgent* ExSequenceAgent = Cast<UExSequenceAgent>(GetOuter());
	if (!bHasBeenInitialized && !HasAnyFlags(ExcludeFlags) && ExSequenceAgent && !ExSequenceAgent->HasAnyFlags(ExcludeFlags))
	{
		UObject* ContextObject = ExSequenceAgent->GetOuter();

		// FGuid BindingID = MovieScene->AddPossessable(ContextObject ? ContextObject->GetName() : TEXT("Owner"), ContextObject ? ContextObject->GetClass() : UObject::StaticClass());
		FGuid BindingID = MovieScene->AddPossessable(ContextObject ? TEXT("Owner(") + ContextObject->GetClass()->GetName() + TEXT(")") : TEXT("Owner"), ContextObject ? ContextObject->GetClass() : UObject::StaticClass());
		ObjectReferences.CreateBinding(BindingID, FExSequenceObjectReference::CreateForContextObject());

		const bool bFrameLocked = CVarDefaultEvaluationType.GetValueOnGameThread() != 0;

		MovieScene->SetEvaluationType(bFrameLocked ? EMovieSceneEvaluationType::FrameLocked : EMovieSceneEvaluationType::WithSubFrames);

		FFrameRate TickResolution(60000, 1);
		TryParseString(TickResolution, *CVarDefaultTickResolution.GetValueOnGameThread());
		MovieScene->SetTickResolutionDirectly(TickResolution);

		FFrameRate DisplayRate(30, 1);
		TryParseString(DisplayRate, *CVarDefaultDisplayRate.GetValueOnGameThread());
		MovieScene->SetDisplayRate(DisplayRate);

		OnInitializeSequenceEvent.Broadcast(this);
		bHasBeenInitialized = true;
	}
#endif

	Super::PostInitProperties();
}

void UExSequence::BindPossessableObject(const FGuid& ObjectId, UObject& PossessedObject, UObject* Context)
{
	if (!Context)
	{
		return;
	}

	if (UActorComponent* Component = Cast<UActorComponent>(&PossessedObject))
	{
		ObjectReferences.CreateBinding(ObjectId, FExSequenceObjectReference::CreateForComponent(Component));
	}
	else
	{
		ObjectReferences.CreateBinding(ObjectId, FExSequenceObjectReference::CreateForObject(&PossessedObject, Context));
	}
}

bool UExSequence::CanPossessObject(UObject& Object, UObject* InPlaybackContext) const
{
	if (InPlaybackContext == nullptr)
	{
		return false;
	}

	AActor* ActorContext = Cast<AActor>(InPlaybackContext);

	if (ActorContext)
	{
		if (AActor* Actor = Cast<AActor>(&Object))
		{
			return Actor == InPlaybackContext || Actor->GetLevel() == ActorContext->GetLevel();
		}
		else if (UActorComponent* Component = Cast<UActorComponent>(&Object))
		{
			return Component->GetOwner() ? Component->GetOwner()->GetLevel() == ActorContext->GetLevel() : false;
		}
	}

	return false;
}

void UExSequence::LocateBoundObjects(const FGuid& ObjectId, UObject* Context, TArray<UObject*, TInlineAllocator<1>>& OutObjects) const
{
	if (Context)
	{
		ObjectReferences.ResolveBinding(ObjectId, Context, OutObjects);
	}
}

UMovieScene* UExSequence::GetMovieScene() const
{
	return MovieScene;
}

UObject* UExSequence::GetParentObject(UObject* Object) const
{
	if (UActorComponent* Component = Cast<UActorComponent>(Object))
	{
		return Component->GetOwner();
	}

	return nullptr;
}

void UExSequence::UnbindPossessableObjects(const FGuid& ObjectId)
{
	ObjectReferences.RemoveBinding(ObjectId);
}

UObject* UExSequence::CreateDirectorInstance(IMovieScenePlayer& Player)
{
	AActor* Actor = Cast<AActor>(Player.GetPlaybackContext());
	if (!Actor)
	{
		return Player.GetPlaybackContext();
	}

	// If this sequence is inside a blueprint, or its component's archetype is from a blueprint, we use the actor as the instace (which will be an instance of the blueprint itself)
	if (GetTypedOuter<UBlueprintGeneratedClass>() || GetTypedOuter<UExSequenceAgent>()->GetArchetype() != GetDefault<UExSequenceAgent>())
	{
		return Actor;
	}


	// Otherwise we use the level script actor as the instance
	return Actor->GetLevel()->GetLevelScriptActor();
}

#if WITH_EDITOR
FText UExSequence::GetDisplayName() const
{
	UExSequenceAgent* ExSequenceAgent = GetTypedOuter<UExSequenceAgent>();

	if (ExSequenceAgent)
	{
		FString OwnerName;

		if (UBlueprint* Blueprint = GetParentBlueprint())
		{
			OwnerName = Blueprint->GetName();
		}
		else if (UObject* Outer = ExSequenceAgent->GetOuter())
		{
			OwnerName = Outer->GetName();
		}

		return OwnerName.IsEmpty()
			? FText::FromName(ExSequenceAgent->GetFName())
			: FText::Format(NSLOCTEXT("ExSequence", "DisplayName", "{0} ({1})"), FText::FromName(ExSequenceAgent->GetFName()), FText::FromString(OwnerName));
	}

	return UMovieSceneSequence::GetDisplayName();
}


ETrackSupport UExSequence::IsTrackSupported(TSubclassOf<class UMovieSceneTrack> InTrackClass) const
{
	if (InTrackClass == UMovieSceneAudioTrack::StaticClass() ||
		//InTrackClass == UMovieSceneCameraCutTrack::StaticClass() ||
		//InTrackClass == UMovieSceneCinematicShotTrack::StaticClass() ||
		InTrackClass == UMovieSceneEventTrack::StaticClass() ||
		InTrackClass == UMovieSceneFadeTrack::StaticClass() ||
		InTrackClass == UMovieSceneLevelVisibilityTrack::StaticClass() ||
		InTrackClass == UMovieSceneMaterialParameterCollectionTrack::StaticClass() ||
		//InTrackClass == UMovieSceneSlomoTrack::StaticClass() ||
		InTrackClass == UMovieSceneSpawnTrack::StaticClass() ||
		InTrackClass == UMovieSceneSubTrack::StaticClass())
	{
		return ETrackSupport::Supported;
	}

	return Super::IsTrackSupported(InTrackClass);
}

#endif