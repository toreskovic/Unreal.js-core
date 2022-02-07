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

		if (UWorld* world = GetWorld())
		{
			if (UJavascriptSubsystem* subsystem = world->GetSubsystem<UJavascriptSubsystem>())
			{
				subsystem->DeserializeObject(JavascriptChildActor->GetChildActor(), JavascriptSerializationData);
			}
		}
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
	
#if WITH_EDITOR
	if (UWorld* world = GetWorld())
	{
		if (!world->IsGameWorld())
		{
			FEditorDelegates::PreSaveWorld.AddUObject(this, &AJavascriptProxyActor::OnPreSaveWorld);
		}
	}
#endif
}

void AJavascriptProxyActor::BeginPlay()
{
	Super::BeginPlay();

	AActor* javascriptActor = GetWorld()->SpawnActor(GetActorClass(), &GetTransform());

	if (javascriptActor != nullptr)
	{
		if (UWorld* world = GetWorld())
		{
			if (UJavascriptSubsystem* subsystem = world->GetSubsystem<UJavascriptSubsystem>())
			{
				subsystem->DeserializeObject(javascriptActor, JavascriptSerializationData);
			}
		}
	}
}

UClass* AJavascriptProxyActor::GetActorClass() const
{
	if (UWorld* world = GetWorld())
	{
		if (UJavascriptSubsystem* subsystem = world->GetSubsystem<UJavascriptSubsystem>())
		{
			return subsystem->GetJavascriptActorClass(*JavascriptClassName);
		}
	}

	return nullptr;
}

void AJavascriptProxyActor::OnPreSaveWorld(uint32 SaveFlags, UWorld* World)
{
	if (World)
	{
		if (UJavascriptSubsystem* subsystem = World->GetSubsystem<UJavascriptSubsystem>())
		{
			if (AActor* childActor = JavascriptChildActor->GetChildActor())
			{
				 JavascriptSerializationData = subsystem->SerializeObject(childActor);
			}
		}
	}
}
