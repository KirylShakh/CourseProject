// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldActor.h"
#include "../StoryModeProjectCharacter.h"

#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

AShieldActor::AShieldActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneComponent;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	BoxComponent->SetupAttachment(RootComponent);
	BoxComponent->SetRelativeLocation(FVector(0.f, 0.f, -50.f));
	BoxComponent->SetWorldScale3D(FVector(.1f, .5f, 1.f));
	BoxComponent->SetBoxExtent(FVector(32.f, 128.f, 128.f));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(BoxComponent);
	Mesh->SetWorldScale3D(FVector(1.f, 3.2f, 2.8f));
}

void AShieldActor::BeginPlay()
{
	Super::BeginPlay();

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AShieldActor::OnTrigger);
}

void AShieldActor::OnTrigger_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Cast<AWeaponActor>(OtherActor) || OtherActor->GetOwner() == GetOwner())
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		if (!CrumbleTimerHandle.IsValid())
		{
			GetWorldTimerManager().SetTimer(CrumbleTimerHandle, this, &AShieldActor::OnCrumbleTimerCompleted, CrumbleTime, false);
		}
	}
}

void AShieldActor::OnCrumbleTimerCompleted()
{
	BoxComponent->OnComponentBeginOverlap.RemoveDynamic(this, &AShieldActor::OnTrigger);

	if (CrumbleTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(CrumbleTimerHandle);
	}

	AStoryModeProjectCharacter* Character = Cast<AStoryModeProjectCharacter>(GetOwner());
	if (Character)
	{
		Character->ShieldDestroyed();
	}
}

void AShieldActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AStoryModeProjectCharacter* Character = Cast<AStoryModeProjectCharacter>(GetOwner());
	if (Character)
	{
		const FVector CharacterForwardVector = Character->GetActorForwardVector();

		const FVector SpawnLocation = Character->GetMuzzlePoint() + CharacterForwardVector * Offset;
		const FRotator SpawnRotation = CharacterForwardVector.Rotation();
		const FVector SpawnScale = FVector(1.f);

		SetActorTransform(FTransform(SpawnRotation, SpawnLocation, SpawnScale));
	}
}

void AShieldActor::HitByOtherWeapon(AWeaponActor* OtherWeapon)
{
	if (GetOwner()->HasAuthority())
	{
		if (!CrumbleTimerHandle.IsValid())
		{
			GetWorldTimerManager().SetTimer(CrumbleTimerHandle, this, &AShieldActor::OnCrumbleTimerCompleted, CrumbleTime, false);
		}
	}
}
