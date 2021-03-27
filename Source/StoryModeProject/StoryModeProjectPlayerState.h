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

	UPROPERTY(ReplicatedUsing = OnRep_CharacterClass, EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AStoryModeProjectCharacter> CharacterClass;

	UPROPERTY(ReplicatedUsing = OnRep_KillCount, BlueprintReadWrite)
	int32 KillCount = 0;

	UFUNCTION(BlueprintImplementableEvent)
	void OnUIDataChanged();

	UFUNCTION(BlueprintImplementableEvent)
	void OnCharacterClassChanged();

protected:
	UFUNCTION()
	virtual void OnRep_Name();

	UFUNCTION()
	virtual void OnRep_Avatar();

	UFUNCTION()
	virtual void OnRep_KillCount();

	UFUNCTION()
	virtual void OnRep_CharacterClass();

	// Used to copy properties from the current PlayerState to the passed one
	virtual void CopyProperties(APlayerState* PlayerState) override;
	// Used to override the current PlayerState with the properties of the passed one
	virtual void OverrideWith(APlayerState* PlayerState) override;

	void CopyInto(AStoryModeProjectPlayerState* PlayerState);
};
