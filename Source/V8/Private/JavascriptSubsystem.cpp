#include "JavascriptSubsystem.h"
#include "V8/Public/JavascriptIsolate.h"
#include "V8/Public/JavascriptContext.h"
#include "V8/Public/JavascriptSettings.h"

void UJavascriptSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
    Super::Initialize(Collection);
    
    UJavascriptContext::JavascriptActorClassesMap.Reset();

    UJavascriptIsolate *Isolate = nullptr;
    if (!IsRunningCommandlet())
    {
        UJavascriptStaticCache *StaticGameData = Cast<UJavascriptStaticCache>(GEngine->GameSingleton);
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

    auto *Context = Isolate->CreateContext();
#if WITH_EDITOR
    //Context->CreateInspector();
#endif

    JavascriptContext = Context;
    JavascriptIsolate = Isolate;

    Context->Expose("Root", this);
    Context->Expose("GWorld", GetWorld());
    Context->Expose("GEngine", GEngine);

    if (UWorld* world = GetWorld())
    {
        if (!world->IsGameWorld())
        {
            JavascriptContext->RunFile(TEXT("index.js"));
        }
        else
        {
			world->OnActorsInitialized.AddLambda([&](const UWorld::FActorsInitializedParams& P)
			{
                JavascriptContext->RunFile(TEXT("index.js"));
			});
        }
    }

    UE_LOG(Javascript, Log, TEXT("Javascript Subsystem Initialized"));
}

void UJavascriptSubsystem::Deinitialize()
{
#if WITH_EDITOR
    if (JavascriptContext)
    {
        //JavascriptContext->DestroyInspector();
    }
#endif

    if (IsValid(GEngine) && !IsRunningCommandlet())
    {
        auto *StaticGameData = Cast<UJavascriptStaticCache>(GEngine->GameSingleton);
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

    Super::Deinitialize();
}

void UJavascriptSubsystem::Tick(float DeltaTime)
{
    if (LastFrameNumberWeTicked == GFrameCounter)
        return;

	OnTick.ExecuteIfBound(DeltaTime);

    LastFrameNumberWeTicked = GFrameCounter;
}

UJavascriptContext *UJavascriptSubsystem::GetJavascriptContext() const
{
    return JavascriptContext;
}

UObject *UJavascriptSubsystem::ResolveAsset(FName Name, bool bTryLoad)
{
    const UJavascriptSettings *settings = GetDefault<UJavascriptSettings>();

    if (settings->ResolvableAssetsClassesDT != nullptr)
    {
        FJavascriptResolvableAssetsClassesRow *row = settings->ResolvableAssetsClassesDT->FindRow<FJavascriptResolvableAssetsClassesRow>(Name, TEXT("UJavascriptComponent::ResolveAsset"), true);
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

UClass *UJavascriptSubsystem::ResolveClass(FName Name)
{
    const UJavascriptSettings *settings = GetDefault<UJavascriptSettings>();

    if (settings->ResolvableAssetsClassesDT != nullptr)
    {
        FJavascriptResolvableAssetsClassesRow *row = settings->ResolvableAssetsClassesDT->FindRow<FJavascriptResolvableAssetsClassesRow>(Name, TEXT("UJavascriptComponent::ResolveAsset"), true);
        if (row == nullptr)
        {
            UE_LOG(Javascript, Error, TEXT("UJavascriptComponent::ResolveAsset | No row found for %s"), *Name.ToString());
        }
        else
        {
            return row->Class;
        }
    }

    return nullptr;
}

UEngine* UJavascriptSubsystem::GetEngine()
{
    return GEngine;
}
