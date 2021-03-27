// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldActor.h"
#include "../StoryModeProjectCharacter.h"

#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"

AShieldActor::AShieldActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneComponent;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComponent->SetupAttachment(RootComponent);
}

void AShieldActor::BeginPlay()
{
	Super::BeginPlay();

	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AShieldActor::OnTrigger);
}

void AShieldActor::OnTrigger_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Cast<AWeaponActor>(OtherActor) || OtherActor->GetOwner() == GetOwner())
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		GetWorldTimerManager().SetTimer(CrumbleTimerHandle, this, &AShieldActor::OnCrumbleTimerCompleted, CrumbleTime, false);
		CapsuleComponent->OnComponentBeginOverlap.RemoveDynamic(this, &AShieldActor::OnTrigger);
	}
}

void AShieldActor::OnCrumbleTimerCompleted()
{
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
