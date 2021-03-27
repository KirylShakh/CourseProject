// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponActor.h"
#include "ShrapnelMissileActor.generated.h"

/**
 *
 */
UCLASS()
class STORYMODEPROJECT_API AShrapnelMissileActor : public AWeaponActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AShrapnelMissileActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* SceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* ProjectileMovement;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	float LaunchTime = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	float LaunchSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	float FlyTime = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	float FlySpeed = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	int32 ShrapnelCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	float SpreadAngle = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	TSubclassOf<class AShrapnelActor> ShrapnelClass;

	float TimePassed = 0.f;

	FVector OwnerLastLocation;

	FVector LaunchLocation;

	FVector LaunchDestination;

	FVector LaunchDirection;

	int32 SpawnedShrapnel = 0;

	UFUNCTION(BlueprintNativeEvent, Category = Collision)
	void OnTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void OnTrigger_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	enum Phase
	{
		PHASE_Launch = 0,
		PHASE_Fly,
		PHASE_Shrapnel,
		PHASE_MAX,
	};
	Phase CurrentPhase = PHASE_Launch;

	UFUNCTION(BlueprintImplementableEvent)
	void OnMoved(const FVector Offset);

	UFUNCTION(BlueprintImplementableEvent)
	void OnLaunched();

	UFUNCTION(BlueprintImplementableEvent)
	void OnImpact(AActor* Actor);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OnDetachedFromCharacter() override;
};
