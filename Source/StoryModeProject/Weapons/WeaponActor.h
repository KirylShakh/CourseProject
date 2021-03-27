// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponActor.generated.h"

UCLASS()
class STORYMODEPROJECT_API AWeaponActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWeaponActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	float Damage = 10.f;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnStartFiring();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnStopFiring();

	virtual void OnDetachedFromCharacter();

	UFUNCTION(BlueprintCallable)
	virtual float GetDamage();
};
