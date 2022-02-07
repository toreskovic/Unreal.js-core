#pragma once

#include "V8PCH.h"
#include "JavascriptSubsystem.h"
#include "JavascriptProxyActor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class AJavascriptProxyActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category=ChildActorComponent, meta=(ShowInnerProperties))
	AActor* JavascriptChildActor;

	UPROPERTY(EditDefaultsOnly)
	FString JavascriptClassName;

	AJavascriptProxyActor();

	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void RefreshJavascriptActor();

	virtual void PostInitProperties() override;

	virtual void PostRegisterAllComponents() override;

	virtual void BeginPlay() override;

private:
	UClass* GetActorClass() const;

	UFUNCTION()
	void OnPreSaveWorld(uint32 SaveFlags, UWorld* World);

	UPROPERTY()
	FJavascriptSerializationData JavascriptSerializationData;
};