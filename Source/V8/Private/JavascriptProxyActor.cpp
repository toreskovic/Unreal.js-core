#include "JavascriptProxyActor.h"
#include "JavascriptComponent.h"
#include "JavascriptSubsystem.h"

AJavascriptProxyActor::AJavascriptProxyActor()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	JavascriptChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("JavascriptChildActor"));
	JavascriptChildActor->SetupAttachment(RootComponent);

	JavascriptChildActor->bIsEditorOnly = true;
}

void AJavascriptProxyActor::RefreshJavascriptActor()
{
#if WITH_EDITORONLY_DATA
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		JavascriptChildActor->SetChildActorClass(GetActorClass(), nullptr);
	}
#endif
}

void AJavascriptProxyActor::PostInitProperties()
{
	Super::PostInitProperties();
}

void AJavascriptProxyActor::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	RefreshJavascriptActor();
}

void AJavascriptProxyActor::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->SpawnActor(GetActorClass(), &GetTransform());
}

UClass* AJavascriptProxyActor::GetActorClass() const
{
	if (UWorld* world = GetWorld())
	{
		if (UJavascriptSubsystem* subsystem = world->GetSubsystem<UJavascriptSubsystem>())
		{
			if (TWeakObjectPtr<UClass>* weakClassPtr = UJavascriptContext::JavascriptActorClassesMap.Find(*JavascriptClassName))
			{
				TWeakObjectPtr<UClass> weakClass = *weakClassPtr;
				if (weakClass.IsValid())
				{
					return weakClass.Get();
				}
			}
		}
	}

	return nullptr;
}
