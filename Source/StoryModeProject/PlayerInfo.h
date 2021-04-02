// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerInfo.generated.h"

USTRUCT(BlueprintType)
struct FPlayerInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FText Name = FText();

	UPROPERTY(BlueprintReadWrite)
	UTexture2D* Avatar = nullptr;

	UPROPERTY(BlueprintReadWrite)
	int32 KillCount = 0;

	UPROPERTY(BlueprintReadWrite)
	FVector2D ScreenLocation = FVector2D(0.f);
};
