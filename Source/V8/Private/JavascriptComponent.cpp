#include "JavascriptComponent.h"
#include "JavascriptIsolate.h"
#include "JavascriptContext.h"
#include "JavascriptStats.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "V8PCH.h"
#include "IV8.h"
#include "V8/Public/JavascriptSettings.h"


DECLARE_CYCLE_STAT(TEXT("Javascript Component Tick Time"), STAT_JavascriptComponentTickTime, STATGROUP_Javascript);

UJavascriptComponent::UJavascriptComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = false;
	bAutoActivate = true;
	bWantsInitializeComponent = true;
}

void UJavascriptComponent::OnRegister()
{
	auto ContextOwner = GetOuter();
	if (ContextOwner && !HasAnyFlags(RF_ClassDefaultObject) && !ContextOwner->HasAnyFlags(RF_ClassDefaultObject))
	{
		if (GetWorld() && ((GetWorld()->IsGameWorld() && !GetWorld()->IsPreviewWorld()) || bActiveWithinEditor))
		{
			UJavascriptIsolate* Isolate = nullptr;
			if (!IsRunningCommandlet())
			{
				UJavascriptStaticCache* StaticGameData = Cast<UJavascriptStaticCache>(GEngine->GameSingleton);
				if (StaticGameData)
				{
					if (StaticGameData->Isolates.Num() > 0)
						Isolate = StaticGameData->Isolates.Pop();
				}
			}

			if (!Isolate)
			{
				Isolate = NewObject<UJavascriptIsolate>();
				Isolate->Init(false);
				Isolate->AddToRoot();
			}

			auto* Context = Isolate->CreateContext();
#if WITH_EDITOR
			//Context->CreateInspector();
#endif

			JavascriptContext = Context;
			JavascriptIsolate = Isolate;

			Context->Expose("Root", this);
			Context->Expose("GWorld", GetWorld());
			Context->Expose("GEngine", GEngine);
		}
	}

	Super::OnRegister();
}

void UJavascriptComponent::Activate(bool bReset)
{
	Super::Activate(bReset);

	if (JavascriptContext)
	{
		JavascriptContext->RunFile(*ScriptSourceFile);

		SetComponentTickEnabled(OnTick.IsBound());
	}

	OnBeginPlay.ExecuteIfBound();
}

void UJavascriptComponent::Deactivate()
{
	OnEndPlay.ExecuteIfBound();

	Super::Deactivate();
}

void UJavascriptComponent::BeginDestroy()
{
	if (IsValid(GEngine) && !IsRunningCommandlet())
	{
		auto* StaticGameData = Cast<UJavascriptStaticCache>(GEngine->GameSingleton);
		if (StaticGameData)
		{
			StaticGameData->Isolates.Add(JavascriptIsolate);
		}
		else if (JavascriptIsolate)
		{
			JavascriptIsolate->RemoveFromRoot();
			JavascriptIsolate = nullptr;
			JavascriptContext = nullptr;
		}
	}
	if (IsActive())
	{
		Deactivate();
	}

	Super::BeginDestroy();
}

void UJavascriptComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	check(bRegistered);

	SCOPE_CYCLE_COUNTER(STAT_JavascriptComponentTickTime);

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	OnTick.ExecuteIfBound(DeltaTime);
}

void UJavascriptComponent::ForceGC()
{
	JavascriptContext->RequestV8GarbageCollection();
}

void UJavascriptComponent::Expose(FString ExposedAs, UObject* Object)
{
	JavascriptContext->Expose(ExposedAs, Object);
}

void UJavascriptComponent::Invoke(FName Name)
{
	OnInvoke.ExecuteIfBound(Name);
}

void UJavascriptComponent::ProcessEvent(UFunction* Function, void* Parms)
{
	if (JavascriptContext && JavascriptContext->CallProxyFunction(this, this, Function, Parms))
	{
		return;
	}

	Super::ProcessEvent(Function, Parms);
}

UObject* UJavascriptComponent::ResolveAsset(FName Name, bool bTryLoad)
{
	const UJavascriptSettings* settings = GetDefault<UJavascriptSettings>();

	if (settings->ResolvableAssetsClassesDT != nullptr)
	{
		FJavascriptResolvableAssetsClassesRow* row = settings->ResolvableAssetsClassesDT->FindRow<FJavascriptResolvableAssetsClassesRow>(Name, TEXT("UJavascriptComponent::ResolveAsset"), true);
		if (row == nullptr)
		{
			UE_LOG(Javascript, Error, TEXT("UJavascriptComponent::ResolveAsset | No row found for %s"), *Name.ToString());
		}
		else
		{
			return bTryLoad ? row->Asset.TryLoad() : row->Asset.ResolveObject();
		}
	}

	return nullptr;
}

UClass* UJavascriptComponent::ResolveClass(FName Name)
{
	const UJavascriptSettings* settings = GetDefault<UJavascriptSettings>();

	if (settings->ResolvableAssetsClassesDT != nullptr)
	{
		FJavascriptResolvableAssetsClassesRow* row = settings->ResolvableAssetsClassesDT->FindRow<FJavascriptResolvableAssetsClassesRow>(Name, TEXT("UJavascriptComponent::ResolveAsset"), true);
		if (row == nullptr)
		{
			UE_LOG(Javascript, Error, TEXT("UJavascriptComponent::ResolveAsset | No row found for %s"), *Name.ToString());
		}
		else
		{
			return row->Class.LoadSynchronous();
		}
	}

	return nullptr;
}

