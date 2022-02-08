#include "JavascriptProxyActor.h"
#include "JavascriptComponent.h"
#include "JavascriptSubsystem.h"
#include "JavascriptContext_Private.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

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
			if (!JavascriptChildActor.IsValid() || JavascriptChildActor->GetClass() != GetActorClass())
			{
				if (JavascriptChildActor.IsValid())
				{
					JavascriptChildActor->Destroy();
				}

				FActorSpawnParameters Params;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				Params.bAllowDuringConstructionScript = true;
				Params.OverrideLevel = GetLevel();
				Params.Owner = this;
				Params.bTemporaryEditorActor = true;

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
				subsystem->DeserializeObject(JavascriptChildActor.Get(), JavascriptSerializationData);

				if (UJavascriptContext* context = subsystem->GetJavascriptContext())
				{
					context->JavascriptContext->CallProxyFunction(GetActorClass(), JavascriptChildActor.Get(), TEXT("postProxySpawn"), nullptr);
				}
			}
		}
	}
#endif
}

void AJavascriptProxyActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	if (!HasAnyFlags(RF_ClassDefaultObject | RF_Transient))
	{
		if (UWorld* world = GetWorld())
		{
			if (!world->IsGameWorld())
			{
				RefreshJavascriptActor();

				FEditorDelegates::PreSaveWorld.AddUObject(this, &AJavascriptProxyActor::OnPreSaveWorld);
			}
		}
	}
#endif
}

void AJavascriptProxyActor::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* world = GetWorld())
	{
		AActor* javascriptActor = GetWorld()->SpawnActorDeferred<AActor>(GetActorClass(), GetTransform());

		if (javascriptActor != nullptr)
		{
			if (UJavascriptSubsystem* subsystem = world->GetSubsystem<UJavascriptSubsystem>())
			{
				subsystem->DeserializeObject(javascriptActor, JavascriptSerializationData);

				UGameplayStatics::FinishSpawningActor(javascriptActor, JavascriptSerializationData.ActorTransform);
				if (UJavascriptContext* context = subsystem->GetJavascriptContext())
				{
					context->JavascriptContext->CallProxyFunction(GetActorClass(), javascriptActor, TEXT("postProxySpawn"), nullptr);
				}
			}
		}
	}
	}
}

void AJavascriptProxyActor::BeginDestroy()
{
	if (JavascriptChildActor.IsValid())
	{
		JavascriptChildActor->Destroy();
	}

	Super::BeginDestroy();
}

void AJavascriptProxyActor::Serialize(FArchive& Ar)
{
	if (Ar.IsSaving())
	{
		if (UWorld* world = GetWorld())
		{
			if (UJavascriptSubsystem* subsystem = world->GetSubsystem<UJavascriptSubsystem>())
			{
				JavascriptSerializationData = subsystem->SerializeObject(JavascriptChildActor.Get());
			}
		}
	}

	Super::Serialize(Ar);
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

