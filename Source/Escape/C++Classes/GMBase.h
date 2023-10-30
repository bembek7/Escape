// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Room.h"
#include "GameFramework/Character.h"

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
	UFUNCTION(BlueprintCallable, Category = "Rooms")
	void FloorCompleted();

	UFUNCTION()
	bool PlayedTutorial() const;

	UFUNCTION(BlueprintCallable, Category = "Rooms")
	FString GetSaveSlotName() const;

	UFUNCTION(BlueprintCallable, Category = "Rooms")
	int32 GetSaveIndex() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Rooms");
	uint32 NumberOfRoomsToSpawn = 5;
	UPROPERTY(EditDefaultsOnly, Category = "Rooms");
	TArray<TSubclassOf<ARoom>>RoomsClasses;
	UPROPERTY(EditDefaultsOnly, Category = "Save");
	uint32 SaveIndex = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Save");
	FString SaveSlotName = "DefaultSave";
	UPROPERTY(EditDefaultsOnly, Category = "Rooms");
	TSubclassOf<ARoom>FinalRoomClass;

private:
	UFUNCTION()
	void PlayerDestroyed(AActor* DestroyedPlayer);

	UFUNCTION(Category = "Rooms")
	void ClearRooms();

	UFUNCTION(Category = "Rooms")
	TSubclassOf<ARoom> GetRandomRoomClass();
	
	UFUNCTION(Category = "Rooms")
	uint32 GetRandomRoomIndex(); // gets random room index, randomizing is make that so rooms wouldn't reapet and appear regularly  

	UFUNCTION(Category = "Rooms")
	void UpdateChances(uint32 ChosenIndex); // updating chances after picking the room to spawn

	UFUNCTION(Category = "Rooms")
	void GenerateEqualChances();

	UFUNCTION(Category = "Rooms")
	void SpawnRooms(uint32 RoomsToSpawn, const FTransform& LastExitTransform);

	UFUNCTION(Category = "Rooms")
	void SpawnRoom(const TSubclassOf<ARoom>& RoomClass, const FTransform& SpawnTransform);

	TArray<uint32>RoomsChancesOfSpawning;
	TArray<ARoom*>SpawnedRooms;
	FTransform LastRoomExitTransform;
	uint32 SpawnedLeftTurns = 0;
	uint32 SpawnedRightTurns = 0;
};
