// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponActor.h"
#include "ShieldActor.generated.h"

/**
 *
 */
UCLASS()
class STORYMODEPROJECT_API AShieldActor : public AWeaponActor
{
	GENERATED_BODY()

public:
	AShieldActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* SceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* CapsuleComponent;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	float Offset = 100.f;

	UFUNCTION(BlueprintNativeEvent, Category = Collision)
	void OnTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void OnTrigger_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	float CrumbleTime = .5f;

	UPROPERTY()
	FTimerHandle CrumbleTimerHandle;

	UFUNCTION()
	void OnCrumbleTimerCompleted();

public:
	virtual void Tick(float DeltaTime) override;
};
