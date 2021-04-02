// Fill out your copyright notice in the Description page of Project Settings.


#include "StoryModeProjectPlayerController.h"
#include "StoryModeProjectPlayerState.h"
#include "StoryModeProjectCharacter.h"
#include "ArenaGameState.h"

#include "GameFramework/GameStateBase.h"

TArray<FPlayerInfo> AStoryModeProjectPlayerController::EnemyInfo()
{
    TArray<FPlayerInfo> Result;

    AArenaGameState* const GameState = GetWorld()->GetGameState<AArenaGameState>();
    if (GameState)
    {
        for (int i = 0; i < GameState->Players.Num() || i < GameState->Bots.Num(); i++)
        {
            if (i < GameState->Players.Num())
            {
                AStoryModeProjectPlayerState* EnemyPlayerState = GameState->Players[i];
                if (EnemyPlayerState)
                {
                    AStoryModeProjectCharacter* Enemy = Cast<AStoryModeProjectCharacter>(EnemyPlayerState->GetPawn());
                    if (Enemy && !Enemy->IsDead() && Enemy != GetPawn<AStoryModeProjectCharacter>())
                    {
                        FPlayerInfo Info = Enemy->GetPlayerInfo();
                        ProjectWorldLocationToScreen(Enemy->GetMuzzlePoint(), Info.ScreenLocation);

                        Result.Add(Info);
                    }
                }
            }

            if (i < GameState->Bots.Num())
            {
                AStoryModeProjectCharacter* Enemy = Cast<AStoryModeProjectCharacter>(GameState->Bots[i]);
                if (Enemy && !Enemy->IsDead() && Enemy != GetPawn<AStoryModeProjectCharacter>())
                {
                    FPlayerInfo Info = Enemy->GetPlayerInfo();
                    ProjectWorldLocationToScreen(Enemy->GetMuzzlePoint(), Info.ScreenLocation);

                    Result.Add(Info);
                }
            }
        }
    }

    return Result;
}
