// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveGameBase.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPE_API USaveGameBase : public USaveGame
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SaveFloorCount(int32 NewFloorCount);

	UFUNCTION(BlueprintCallable)
	int32 GetBestFloorCount();

	UFUNCTION(BlueprintCallable)
	void SaveTime(int32 NewTime);

	UFUNCTION(BlueprintCallable)
	int32 GetBestTime();

	bool bPlayedTutorial = false;
private:
	int32 BestFloorCount = 0;
	int32 BestTime = 9999999;
};
