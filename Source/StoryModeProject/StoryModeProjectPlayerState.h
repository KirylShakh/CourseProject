// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "StoryModeProjectPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class STORYMODEPROJECT_API AStoryModeProjectPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	/** Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_Name, EditAnywhere, BlueprintReadWrite)
	FText Name;

	UPROPERTY(ReplicatedUsing = OnRep_Avatar, EditAnywhere, BlueprintReadWrite)
	UTexture2D* Avatar;

	UPROPERTY(Replicated, BlueprintReadWrite)
	int32 KillCount = 0;

	UFUNCTION(BlueprintImplementableEvent)
	void OnUIDataChanged();

protected:
	UFUNCTION()
	virtual void OnRep_Name();

	UFUNCTION()
	virtual void OnRep_Avatar();
};
