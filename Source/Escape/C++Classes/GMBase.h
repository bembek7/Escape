// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Room.h"


#include "GMBase.generated.h"
/**
 * 
 */
UCLASS()
class ESCAPE_API AGMBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AGMBase();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Rooms");
	int NumberOfRoomsToSpawn;
	UPROPERTY(EditDefaultsOnly, Category = "Rooms");
	TArray<TSubclassOf<ARoom>>RoomsClasses;
	UPROPERTY(EditDefaultsOnly, Category = "Save");
	int SaveIndex = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Save");
	FString SaveSlotName = "DefaultSave";
	
private:
	UFUNCTION(Category = "Rooms")
	void GenerateEqualChances();

	TArray<int>RoomsChancesOfSpawning;
	TArray<TSubclassOf<ARoom>*>SpawnedRooms;
	FTransform LastRoomExitTransform;
	int SpawnedLeftTurns = 0;
	int SpawnedRightTurns = 0;
};
