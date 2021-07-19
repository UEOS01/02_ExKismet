
#pragma once

#include "UObject/LazyObjectPtr.h"
#include "ExSequenceObjectReference.generated.h"

class UActorComponent;

UENUM()
enum class EExSequenceObjectReferenceType : uint8
{
	/** The reference relates to the context Object. */
	ContextObject,
	/** The reference relates to an Object outside of the context Object. */
	ExternalObject,
	/** The reference relates to a component */
	Component,
};

/**
 * An external reference to an level sequence object, resolvable through an arbitrary context.
 */
USTRUCT()
struct FExSequenceObjectReference
{
	GENERATED_BODY()

	/**
	 * Default construction to a null reference
	 */
	FExSequenceObjectReference()
		: Type(EExSequenceObjectReferenceType::ContextObject)
	{}

	/**
	 * Generates a new reference to an object within a given context.
	 *
	 * @param InComponent The component to create a reference for
	 */
	EXKISMET_API static FExSequenceObjectReference CreateForComponent(UActorComponent* InComponent);

	/**
	 * Generates a new reference to an object within a given context.
	 *
	 * @param InObject The object to create a reference for
	 */
	EXKISMET_API static FExSequenceObjectReference CreateForObject(UObject* InObject, UObject* ResolutionContext);

	/**
	 * Generates a new reference to the root actor.
	 */
	EXKISMET_API static FExSequenceObjectReference CreateForContextObject();

	/**
	 * Check whether this object reference is valid or not
	 */
	bool IsValid() const
	{
		return ObjectId.IsValid() || !PathToComponent.IsEmpty();
	}

	/**
	 * Resolve this reference from the specified source actor
	 *
	 * @return The object
	 */
	EXKISMET_API UObject* Resolve(UObject* SourceObject) const;

	/**
	 * Equality comparator
	 */
	friend bool operator==(const FExSequenceObjectReference& A, const FExSequenceObjectReference& B)
	{
		return A.ObjectId == B.ObjectId && A.PathToComponent == B.PathToComponent;
	}

private:

	/** The type of the binding */
	UPROPERTY()
	EExSequenceObjectReferenceType Type;

	/** The ID of the Object - if this is set, we're either the owner Object, or an object outside of the context */
	UPROPERTY()
	FGuid ObjectId;

	/** Path to the component from its owner Object */
	UPROPERTY()
	FString PathToComponent;
};

USTRUCT()
struct FExSequenceObjectReferences
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FExSequenceObjectReference> Array;
};

USTRUCT()
struct FExSequenceObjectReferenceMap
{
	GENERATED_BODY()

	/**
	 * Check whether this map has a binding for the specified object id
	 * @return true if this map contains a binding for the id, false otherwise
	 */
	bool HasBinding(const FGuid& ObjectId) const;

	/**
	 * Remove a binding for the specified ID
	 *
	 * @param ObjectId	The ID to remove
	 */
	void RemoveBinding(const FGuid& ObjectId);

	/**
	 * Create a binding for the specified ID
	 *
	 * @param ObjectId				The ID to associate the component with
	 * @param ObjectReference	The component reference to bind
	 */
	void CreateBinding(const FGuid& ObjectId, const FExSequenceObjectReference& ObjectReference);

	/**
	 * Resolve a binding for the specified ID using a given context
	 *
	 * @param ObjectId		The ID to associate the object with
	 * @param ParentActor	The parent actor to resolve within
	 * @param OutObjects	Container to populate with bound components
	 */
	void ResolveBinding(const FGuid& ObjectId, UObject* ParentObject, TArray<UObject*, TInlineAllocator<1>>& OutObjects) const;

private:
	
	UPROPERTY()
	TArray<FGuid> BindingIds;

	UPROPERTY()
	TArray<FExSequenceObjectReferences> References;
};
