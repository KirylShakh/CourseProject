// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponActor.h"
#include "LaserActor.generated.h"

UCLASS()
class STORYMODEPROJECT_API ALaserActor : public AWeaponActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALaserActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* SceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* CapsuleComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FTransform RecalculateTransform(FHitResult Hit);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateEffectLocations(const FVector BeamStart, const FVector BeamEnd, bool bValidHit);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	float DamageTickDuration = .5f;

	bool bTimerActive = false;

	UPROPERTY()
	FTimerHandle DamageTickTimerHandle;

	UFUNCTION()
	void OnDamageTickTimerCompleted();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
