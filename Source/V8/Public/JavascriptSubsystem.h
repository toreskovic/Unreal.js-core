#pragma once

#include "V8PCH.h"
#include "JavascriptSubsystem.generated.h"

UCLASS()
class UJavascriptSubsystem : public UWorldSubsystem, public FTickableGameObject
{
    GENERATED_BODY()
public:
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

	UFUNCTION(BlueprintCallable, Category = "Javascript")
	UEngine* GetEngine();

	DECLARE_DYNAMIC_DELEGATE_OneParam(FJavascriptTickSignature, float, DeltaSeconds);	
	UPROPERTY()
	FJavascriptTickSignature OnTick;

private:
	UPROPERTY(transient)
	UJavascriptContext* JavascriptContext;

	UPROPERTY(transient)
	UJavascriptIsolate* JavascriptIsolate;
	
	// The last frame number we were ticked.
	// We don't want to tick multiple times per frame 
	uint32 LastFrameNumberWeTicked = INDEX_NONE;
};