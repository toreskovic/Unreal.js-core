#include "JavascriptProxyActor.h"
#include "JavascriptComponent.h"
#include "JavascriptSubsystem.h"

AJavascriptProxyActor::AJavascriptProxyActor()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AJavascriptProxyActor::RefreshJavascriptActor()
{
#if WITH_EDITORONLY_DATA
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		if (UWorld* world = GetWorld())
		{
			if (!IsValid(JavascriptChildActor) || JavascriptChildActor->GetClass() != GetActorClass())
			{
				FActorSpawnParameters Params;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				Params.bAllowDuringConstructionScript = true;
				Params.OverrideLevel = GetLevel();
				Params.Owner = this;

				Params.ObjectFlags |= (RF_Transient);

				FVector Location = GetActorLocation();
				FRotator Rotation = GetActorRotation();
				JavascriptChildActor = GetWorld()->SpawnActor(GetActorClass(), &Location, &Rotation, Params);

				if (JavascriptChildActor != nullptr)
				{
					if (USceneComponent* ChildRoot = JavascriptChildActor->GetRootComponent())
					{
						ChildRoot->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
					}
				}
			}

			if (UJavascriptSubsystem* subsystem = world->GetSubsystem<UJavascriptSubsystem>())
			{
				subsystem->DeserializeObject(JavascriptChildActor, JavascriptSerializationData);
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
			JavascriptSerializationData = subsystem->SerializeObject(JavascriptChildActor);
		}
	}
}
