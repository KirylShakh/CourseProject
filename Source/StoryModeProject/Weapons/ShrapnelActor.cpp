// Fill out your copyright notice in the Description page of Project Settings.


#include "ShrapnelActor.h"
#include "../StoryModeProjectCharacter.h"

#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AShrapnelActor::AShrapnelActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneComponent;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComponent->SetupAttachment(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bInitialVelocityInLocalSpace = false;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
}

void AShrapnelActor::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(FlyTime);
}

void AShrapnelActor::OnTrigger_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner() || Cast<AWeaponActor>(OtherActor))
	{
		return;
	}

	AStoryModeProjectCharacter* Target = Cast<AStoryModeProjectCharacter>(OtherActor);
	if (Target)
	{
		AStoryModeProjectCharacter* Character = Cast<AStoryModeProjectCharacter>(GetOwner());
		AController* Controller = Character ? Character->Controller : nullptr;

		Target->TakeDamage(GetDamage(), FDamageEvent(), Controller, GetOwner());
	}

	CapsuleComponent->OnComponentBeginOverlap.RemoveDynamic(this, &AShrapnelActor::OnTrigger);
	OnImpact(OtherActor);
}

void AShrapnelActor::Launch(const FVector LaunchDirection)
{
	ProjectileMovement->Velocity = LaunchDirection * Speed;
	ProjectileMovement->Activate();

	if (HasAuthority())
	{
		CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AShrapnelActor::OnTrigger);
	}
}
