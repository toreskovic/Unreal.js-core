#pragma once

#include "Engine/DataTable.h"
#include "JavascriptSettings.generated.h"

USTRUCT(BlueprintType)
struct V8_API FJavascriptResolvableAssetsClassesRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	FStringAssetReference Asset;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UObject> Class;
};

UCLASS(config = Engine, defaultconfig)
class V8_API UJavascriptSettings
	: public UObject
{
	GENERATED_UCLASS_BODY()

public:	
	UPROPERTY(EditAnywhere, config, Category = Javascript, meta = (
		ConsoleVariable = "unrealjs.v8flags", DisplayName = "V8 Flags",
		ToolTip = "V8 Flags. Please refer to V8 documentation"))
	FString V8Flags;	

	void Apply() const;

	UPROPERTY(EditAnywhere, config, Category = Javascript, meta = (
		DisplayName = "Referenced Assets & Classes DT",
		ToolTip = "Used for ResolveAsset and ResolveClass"))
	TSoftObjectPtr<UDataTable> ResolvableAssetsClassesDT = nullptr;
};
