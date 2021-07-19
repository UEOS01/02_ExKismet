
#include "ExSequence/ExSequenceObjectReference.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/Blueprint.h"
#include "UObject/Package.h"

FExSequenceObjectReference FExSequenceObjectReference::CreateForComponent(UActorComponent* InComponent)
{
	check(InComponent);

	FExSequenceObjectReference NewReference;
	NewReference.Type = EExSequenceObjectReferenceType::Component;

	AActor* Actor = InComponent->GetOwner();
	if (Actor)
	{
		NewReference.PathToComponent = InComponent->GetPathName(Actor);
		return NewReference;
	}

	UBlueprintGeneratedClass* GeneratedClass = InComponent->GetTypedOuter<UBlueprintGeneratedClass>();
	if (GeneratedClass && GeneratedClass->SimpleConstructionScript)
	{
		UBlueprint* Blueprint = Cast<UBlueprint>(GeneratedClass->ClassGeneratedBy);
		if (Blueprint)
		{
			for (USCS_Node* Node : GeneratedClass->SimpleConstructionScript->GetAllNodes())
			{
				if (Node->ComponentTemplate == InComponent)
				{
					NewReference.PathToComponent = Node->GetVariableName().ToString();
					return NewReference;
				}
			}
		}
	}

	ensureMsgf(false, TEXT("Unable to find parent actor for component. Reference will be unresolvable."));
	return NewReference;
}

FExSequenceObjectReference FExSequenceObjectReference::CreateForObject(UObject* InObject, UObject* ContextObject)
{
	if (InObject == ContextObject)
	{
		return CreateForContextObject();
	}

	FExSequenceObjectReference NewReference;
	check(InObject && ContextObject && InObject->GetTypedOuter<ULevel>() == ContextObject->GetTypedOuter<ULevel>());

	NewReference.Type = EExSequenceObjectReferenceType::ExternalObject;
	NewReference.ObjectId = FLazyObjectPtr(InObject).GetUniqueID().GetGuid();
	return NewReference;
}

FExSequenceObjectReference FExSequenceObjectReference::CreateForContextObject()
{
	FExSequenceObjectReference NewReference;
	NewReference.Type = EExSequenceObjectReferenceType::ContextObject;
	return NewReference;
}

UObject* FExSequenceObjectReference::Resolve(UObject* SourceObject) const
{
	check(SourceObject);

	switch(Type)
	{
	case EExSequenceObjectReferenceType::ContextObject:
		return SourceObject;

	case EExSequenceObjectReferenceType::ExternalObject:
		if (ObjectId.IsValid())
		{
			// Fixup for PIE
			int32 PIEInstanceID = SourceObject->GetOutermost()->PIEInstanceID;
			FUniqueObjectGuid FixedUpId(ObjectId);
			if (PIEInstanceID != -1)
			{
				FixedUpId = FixedUpId.FixupForPIE(PIEInstanceID);
			}
			
			FLazyObjectPtr LazyPtr;
			LazyPtr = FixedUpId;

			if (UObject* FoundObject = Cast<UObject>(LazyPtr.Get()))
			{
				if (FoundObject->GetTypedOuter<ULevel>() == SourceObject->GetTypedOuter<ULevel>())
				{
					return FoundObject;
				}
			}
		}
		break;

	case EExSequenceObjectReferenceType::Component:
		if (!PathToComponent.IsEmpty())
		{
			return FindObject<UActorComponent>(SourceObject, *PathToComponent);
		}
		break;
	}

	return nullptr;
}

bool FExSequenceObjectReferenceMap::HasBinding(const FGuid& ObjectId) const
{
	return BindingIds.Contains(ObjectId);
}

void FExSequenceObjectReferenceMap::RemoveBinding(const FGuid& ObjectId)
{
	int32 Index = BindingIds.IndexOfByKey(ObjectId);
	if (Index != INDEX_NONE)
	{
		BindingIds.RemoveAtSwap(Index, 1, false);
		References.RemoveAtSwap(Index, 1, false);
	}
}

void FExSequenceObjectReferenceMap::CreateBinding(const FGuid& ObjectId, const FExSequenceObjectReference& ObjectReference)
{
	int32 ExistingIndex = BindingIds.IndexOfByKey(ObjectId);
	if (ExistingIndex == INDEX_NONE)
	{
		ExistingIndex = BindingIds.Num();

		BindingIds.Add(ObjectId);
		References.AddDefaulted();
	}

	References[ExistingIndex].Array.AddUnique(ObjectReference);
}

void FExSequenceObjectReferenceMap::ResolveBinding(const FGuid& ObjectId, UObject* SourceObject, TArray<UObject*, TInlineAllocator<1>>& OutObjects) const
{
	int32 Index = BindingIds.IndexOfByKey(ObjectId);
	if (Index == INDEX_NONE)
	{
		return;
	}

	for (const FExSequenceObjectReference& Reference : References[Index].Array)
	{
		if (UObject* Object = Reference.Resolve(SourceObject))
		{
			OutObjects.Add(Object);
		}
	}
}