// Fill out your copyright notice in the Description page of Project Settings.


#include "LaserActor.h"
#include "../StoryModeProjectCharacter.h"
#include "../StoryModeProjectPlayerState.h"

#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ALaserActor::ALaserActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneComponent;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ALaserActor::BeginPlay()
{
	Super::BeginPlay();


}

FTransform ALaserActor::RecalculateTransform(FHitResult Hit)
{
	AStoryModeProjectCharacter* Character = Cast<AStoryModeProjectCharacter>(GetOwner());
	if (Character)
	{
		const FVector Start = Character->GetMuzzlePoint();
		const FVector End = Hit.IsValidBlockingHit() ? Hit.ImpactPoint : Hit.TraceEnd;

		const FVector SpawnLocation = Start;
		const FRotator SpawnRotation = (End - Start).Rotation();
		const FVector SpawnScale = FVector(1.f);

		return FTransform(SpawnRotation, SpawnLocation, SpawnScale);
	}

	return FTransform();
}

void ALaserActor::OnDamageTickTimerCompleted()
{
	if (DamageTickTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(DamageTickTimerHandle);
	}

	if (GetOwner()->HasAuthority())
	{
		bTimerActive = false;
	}
}

// Called every frame
void ALaserActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AStoryModeProjectCharacter* Character = Cast<AStoryModeProjectCharacter>(GetOwner());
	if (Character)
	{
		const FHitResult Hit = Character->GetAimHitResult();
		const FTransform NewTransform = RecalculateTransform(Hit);
		SetActorTransform(NewTransform);

		const FVector Start = Character->GetMuzzlePoint();
		const FVector End = Hit.IsValidBlockingHit() ? Hit.ImpactPoint : Hit.TraceEnd;
		UpdateEffectLocations(Start, End, Hit.IsValidBlockingHit());

		if (Character->HasAuthority())
		{
			if (Hit.IsValidBlockingHit())
			{
				AStoryModeProjectCharacter* Target = Cast<AStoryModeProjectCharacter>(Hit.GetActor());
				if (Target && !bTimerActive)
				{
					Target->TakeDamage(GetDamage(), FDamageEvent(), Character->GetController(), Character);

					bTimerActive = true;
					GetWorldTimerManager().SetTimer(DamageTickTimerHandle, this, &ALaserActor::OnDamageTickTimerCompleted, DamageTickDuration, false, -1.f);
				}
				else
				{
					AWeaponActor* WeaponActor = Cast<AWeaponActor>(Hit.GetActor());
					if (WeaponActor)
					{
						WeaponActor->HitByOtherWeapon(this);
					}
				}
			}
		}
	}
}
