#pragma once

#include "V8PCH.h"
#include "JavascriptSubsystem.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FJavascriptSerializationData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FTransform ActorTransform;

	UPROPERTY()
	TArray<uint8> Data;

	bool IsEmpty()
	{
		return ActorTransform.Equals(FTransform::Identity) && Data.Num() == 0;
	};
};

class FJavascriptSerializationWriter : public FMemoryWriter
{
public:
	FJavascriptSerializationWriter (TArray<uint8> &InBytes, bool bIsPersistent = false, bool bSetOffset = false, const FName InArchiveName = NAME_None)
		: FMemoryWriter(InBytes, bIsPersistent, bSetOffset, InArchiveName)
	{
	}

	using FArchive::operator<<; // For visibility of the overloads we don't override

	virtual FArchive &operator<<(class UObject *&Obj) override
	{
		// Serialize the FName as a string
		if (IsLoading())
		{
			FString StringName;
			*this << StringName;
			FName N = FName(*StringName);

			Obj = FindObject<UObject>(ANY_PACKAGE, *StringName, false);
			if (Obj == nullptr)
			{
				FStringAssetReference stringAssetRef = StringName;
				Obj = stringAssetRef.TryLoad();
			}
		}
		else
		{
			FString StringName = Obj->GetPathName();
			*this << StringName;
		}
		return *this;
	}

	virtual bool ShouldSkipProperty(const FProperty* InProperty) const override
	{
		EObjectFlags flags = InProperty->GetFlags();
		EPropertyFlags propFlags = InProperty->GetPropertyFlags();

		if (InProperty->HasAnyPropertyFlags(EPropertyFlags::CPF_InstancedReference))
		{
			return true;
		}

		return false;
	}
};

class FJavascriptSerializationReader : public FMemoryReader
{
public:
	explicit FJavascriptSerializationReader(const TArray<uint8> &InBytes, bool bIsPersistent = false)
		: FMemoryReader(InBytes, bIsPersistent)
	{
	}

	using FArchive::operator<<; // For visibility of the overloads we don't override

	virtual FArchive &operator<<(class UObject *&Obj) override
	{
		// Serialize the FName as a string
		if (IsLoading())
		{
			FString StringName;
			*this << StringName;
			FName N = FName(*StringName);

			Obj = FindObject<UObject>(ANY_PACKAGE, *StringName, false);
			if (Obj == nullptr)
			{
				FStringAssetReference stringAssetRef = StringName;
				Obj = stringAssetRef.TryLoad();
			}
		}
		else
		{
			FString StringName = Obj->GetPathName();
			*this << StringName;
		}
		return *this;
	}

	virtual bool ShouldSkipProperty(const FProperty* InProperty) const override
	{
		EObjectFlags flags = InProperty->GetFlags();
		EPropertyFlags propFlags = InProperty->GetPropertyFlags();

		if (InProperty->HasAnyPropertyFlags(EPropertyFlags::CPF_InstancedReference))
		{
			return true;
		}

		return false;
	}
};

UCLASS()
class UJavascriptSubsystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	friend class FJavascriptContextImplementation;

    // Begin USubsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    // End USubsystem

	// FTickableGameObject Begin
	virtual void Tick( float DeltaTime ) override;
	virtual ETickableTickType GetTickableTickType() const override
	{
		return ETickableTickType::Always;
	}
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT( UJavascriptSubsystem, STATGROUP_Tickables );
	}
	virtual bool IsTickableWhenPaused() const
	{
		return true;
	}
	virtual bool IsTickableInEditor() const
	{
		return true;
	}
	// FTickableGameObject End

    UJavascriptContext* GetJavascriptContext() const;

	UFUNCTION(BlueprintCallable, Category = "Javascript")
    UObject* ResolveAsset(FName Name, bool bTryLoad = true);

	UFUNCTION(BlueprintCallable, Category = "Javascript")
    UClass* ResolveClass(FName Name);

	UFUNCTION(BlueprintCallable)
	static FJavascriptSerializationData SerializeObject(UObject *object);

	UFUNCTION(BlueprintCallable)
	void DeserializeObject(UObject* object, FJavascriptSerializationData saveData);

	UFUNCTION(BlueprintCallable, Category = "Javascript")
	UEngine* GetEngine();

	UClass* GetJavascriptActorClass(FName Name);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FJavascriptTickSignature, float, DeltaSeconds);	
	UPROPERTY()
	FJavascriptTickSignature OnTick;

private:
	UPROPERTY(transient)
	UJavascriptContext* JavascriptContext;

	UPROPERTY(transient)
	UJavascriptIsolate* JavascriptIsolate;

	TMap<FName, TWeakObjectPtr<UClass>> JavascriptActorClassesMap;
	
	// The last frame number we were ticked.
	// We don't want to tick multiple times per frame 
	uint32 LastFrameNumberWeTicked = INDEX_NONE;

	void SetJavascriptActorClass(FName Name, UClass* JavascriptClass);
};