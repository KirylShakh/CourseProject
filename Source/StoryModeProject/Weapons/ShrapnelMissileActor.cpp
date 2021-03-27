// Fill out your copyright notice in the Description page of Project Settings.


#include "ShrapnelMissileActor.h"
#include "ShrapnelActor.h"
#include "../StoryModeProjectCharacter.h"

#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

//#include "Kismet/KismetMathLibrary.h"

// Sets default values
AShrapnelMissileActor::AShrapnelMissileActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneComponent;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComponent->SetupAttachment(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bInitialVelocityInLocalSpace = false;
	ProjectileMovement->InitialSpeed = FlySpeed;
	ProjectileMovement->MaxSpeed = FlySpeed;
}

// Called when the game starts or when spawned
void AShrapnelMissileActor::BeginPlay()
{
	Super::BeginPlay();

	AStoryModeProjectCharacter* Character = Cast<AStoryModeProjectCharacter>(GetOwner());
	if (Character)
	{
		LaunchLocation = Character->GetMuzzlePoint();

		FHitResult Hit = Character->GetAimHitResult();
		LaunchDestination = Hit.IsValidBlockingHit() ? Hit.ImpactPoint : Hit.TraceEnd;

		OwnerLastLocation = LaunchLocation;

		SetActorRotation(FRotator(0.f, 0.f, 0.f));
	}
}

void AShrapnelMissileActor::OnTrigger_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner() || Cast<AWeaponActor>(OtherActor))
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		AStoryModeProjectCharacter* Target = Cast<AStoryModeProjectCharacter>(OtherActor);
		if (Target)
		{
			AStoryModeProjectCharacter* Character = Cast<AStoryModeProjectCharacter>(GetOwner());
			AController* Controller = Character ? Character->Controller : nullptr;

			Target->TakeDamage(GetDamage(), FDamageEvent(), Controller, GetOwner());
		}
	}

	CapsuleComponent->OnComponentBeginOverlap.RemoveDynamic(this, &AShrapnelMissileActor::OnTrigger);
	PrimaryActorTick.bCanEverTick = false;
	OnImpact(OtherActor);
}

void AShrapnelMissileActor::Tick(float DeltaTime)
{
	if (CurrentPhase == PHASE_Launch)
	{
		if (TimePassed >= LaunchTime)
		{
			CurrentPhase = PHASE_Fly;
			TimePassed = 0.f;
			return;
		}

		AStoryModeProjectCharacter* Character = Cast<AStoryModeProjectCharacter>(GetOwner());
		if (Character)
		{
			//const FRotator Rotation = Character->Controller->GetControlRotation();
			//const FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Z);
			const FVector Direction = FVector::UpVector;
			const FVector OwnerDelta = Character->GetMuzzlePoint() - OwnerLastLocation;
			const float Speed = LaunchSpeed * (LaunchTime - TimePassed) / LaunchTime;

			const FVector Offset = OwnerDelta + Direction * Speed * DeltaTime;
			SetActorLocation(GetActorLocation() + Offset);

			OwnerLastLocation = Character->GetMuzzlePoint();

			OnMoved(Offset);
		}
	}
	else if (CurrentPhase == PHASE_Fly)
	{
		if (TimePassed >= FlyTime)
		{
			CurrentPhase = PHASE_Shrapnel;
			TimePassed = 0.f;
			return;
		}

		if (TimePassed == 0.f)
		{
			LaunchDirection = (LaunchDestination - GetActorLocation()).GetSafeNormal();
			ProjectileMovement->Velocity = LaunchDirection * FlySpeed;
			ProjectileMovement->Activate();

			CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AShrapnelMissileActor::OnTrigger);

			OnLaunched();
		}
	}
	else if (CurrentPhase == PHASE_Shrapnel)
	{
		if (GetOwner()->HasAuthority())
		{
			if (SpawnedShrapnel < ShrapnelCount)
			{
				FTransform SpawnTransform = FTransform(GetActorRotation(), GetActorLocation(), GetActorScale3D());

				FActorSpawnParameters SpawnParameters;
				SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParameters.Owner = GetOwner();
				SpawnParameters.Instigator = Cast<APawn>(GetOwner());

				AShrapnelActor* ShrapnelActor = GetWorld()->SpawnActor<AShrapnelActor>(ShrapnelClass, SpawnTransform, SpawnParameters);

				const float Pitch = FMath::RandRange(-SpreadAngle, SpreadAngle);
				const float Yaw = FMath::RandRange(-SpreadAngle, SpreadAngle);
				FVector Direction = LaunchDirection.RotateAngleAxis(Pitch, FVector::RightVector);
				Direction = Direction.RotateAngleAxis(Yaw, FVector::UpVector);

				ShrapnelActor->Launch(Direction);

				SpawnedShrapnel += 1;
			}
			else
			{
				Destroy();
			}
		}
		else
		{
			Destroy();
		}
	}

	TimePassed += DeltaTime;
}

void AShrapnelMissileActor::OnDetachedFromCharacter()
{
}
