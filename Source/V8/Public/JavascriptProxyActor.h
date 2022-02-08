#pragma once

#include "V8PCH.h"
#include "JavascriptSubsystem.h"
#include "JavascriptProxyActor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class AJavascriptProxyActor : public AActor
{
	GENERATED_BODY()

public:
	TWeakObjectPtr<AActor> JavascriptChildActor;

	UPROPERTY(EditDefaultsOnly)
	FString JavascriptClassName;

	AJavascriptProxyActor();

	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void RefreshJavascriptActor();

	virtual void PostInitProperties() override;

	virtual void PostRegisterAllComponents() override;

	virtual void PostActorCreated();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

	virtual void PreDuplicate(FObjectDuplicationParameters& DupParams) override;

	virtual void Serialize(FArchive& Ar) override;

private:
	UClass* GetActorClass() const;

	UFUNCTION()
	void OnPreSaveWorld(uint32 SaveFlags, UWorld* World);

	UPROPERTY()
	FJavascriptSerializationData JavascriptSerializationData;
};