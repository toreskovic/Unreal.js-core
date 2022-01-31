#pragma once

#include "V8PCH.h"
#include "JavascriptProxyActor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class V8_API AJavascriptProxyActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient, SkipSerialization)
	UChildActorComponent* JavascriptChildActor;

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
};