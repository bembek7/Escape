// Fill out your copyright notice in the Description page of Project Settings.


#include "GMBase.h"
#include "Kismet/GameplayStatics.h"
#include "SaveGameBase.h"


AGMBase::AGMBase()
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveIndex))
	{
		USaveGame* SaveObject = UGameplayStatics::CreateSaveGameObject(USaveGameBase::StaticClass());
		UGameplayStatics::SaveGameToSlot(SaveObject, SaveSlotName, SaveIndex);
	}
}

void AGMBase::BeginPlay()
{
	Super::BeginPlay();

	RoomsChancesOfSpawning.Init(0, RoomsClasses.Num());
	GenerateEqualChances();

	LastRoomExitTransform = Cast<ARoom>(UGameplayStatics::GetActorOfClass(GetWorld(), ARoom::StaticClass()))->GetExitTransform();
	SpawnRooms(NumberOfRoomsToSpawn, LastRoomExitTransform);
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	PlayerChar->OnDestroyed.AddDynamic(this, &AGMBase::PlayerDestroyed);
}

void AGMBase::FloorCompleted()
{
	SpawnedLeftTurns = 0;
	SpawnedRightTurns = 0;
	// Timer Stuff
	SpawnRooms(NumberOfRoomsToSpawn, LastRoomExitTransform);
}

void AGMBase::PlayerDestroyed(AActor* DestroyedPlayer)
{
	;
}

void AGMBase::ClearRooms()
{
	;
}

TSubclassOf<ARoom> AGMBase::GetRandomRoomClass()
{
	uint32 RandomIndex = GetRandomRoomIndex();
	TSubclassOf<ARoom> RandomClass = RoomsClasses[RandomIndex];
	if (RandomClass.GetDefaultObject()->GetLeftTurns() + SpawnedLeftTurns >= 4 || RandomClass.GetDefaultObject()->GetRightTurns() + SpawnedRightTurns >= 4)
	{
		GetRandomRoomClass();
	}
	return RandomClass;
}

uint32 AGMBase::GetRandomRoomIndex()
{
	TArray<int32>AllRoomsIndexesByChances;
	AllRoomsIndexesByChances.Reserve(100);
	uint32 ChanceIndex = 0;
	for (auto Chance : RoomsChancesOfSpawning)
	{
		for (uint32 i = 0; i < Chance; i++)
		{
			AllRoomsIndexesByChances.Add(ChanceIndex);
		}
		ChanceIndex++;
	}
	int32 RandomizedClassIndex; // the index of room in RoomsClasses Array and Chances Array
	RandomizedClassIndex = AllRoomsIndexesByChances[FMath::RandRange(0, AllRoomsIndexesByChances.Num()-1)];
	UpdateChances(RandomizedClassIndex);
	return RandomizedClassIndex;
}

void AGMBase::UpdateChances(uint32 ChosenIndex)
{
	uint32 ChancesOfChosenIndex = RoomsChancesOfSpawning[ChosenIndex];
	// we will set chances of index we got to 0 so we are distributing the chances of it to other classes chances
	uint32 DistributionOnOther = ChancesOfChosenIndex / (RoomsChancesOfSpawning.Num() - 1);
	uint32 DistributionRest = ChancesOfChosenIndex % (RoomsChancesOfSpawning.Num() - 1);
	uint32 LoopIndex = 0;
	for (auto& Chance : RoomsChancesOfSpawning)
	{
		if (Chance != ChancesOfChosenIndex)
		{
			Chance += DistributionOnOther;
			Chance += DistributionRest;
			DistributionRest = 0;
		}
		else
		{
			Chance = 0;
		}
		LoopIndex++;
	}
}

void AGMBase::GenerateEqualChances()
{
	for (auto& chance : RoomsChancesOfSpawning)
	{
		chance = 100 / RoomsChancesOfSpawning.Num();
	}
	RoomsChancesOfSpawning.Last() += 100 % RoomsChancesOfSpawning.Num();
}

void AGMBase::SpawnRooms(uint32 RoomsToSpawn, const FTransform& LastExitTransform)
{
	if (RoomsToSpawn > 0)
	{
		SpawnRoom(GetRandomRoomClass(), LastExitTransform);
		SpawnRooms(RoomsToSpawn-1, LastRoomExitTransform);
	}
	else
	{
		SpawnRoom(FinalRoomClass, LastExitTransform);
	}
}

void AGMBase::SpawnRoom(const TSubclassOf<ARoom>& RoomClass, const FTransform& SpawnTransform)
{
	ARoom* SpawnedRoom = GetWorld()->SpawnActor<ARoom>(RoomClass.Get(), SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator(), FActorSpawnParameters());
	SpawnedRooms.Add(SpawnedRoom);
	LastRoomExitTransform = Cast<USceneComponent>(SpawnedRoom->GetComponentByClass(ULevelExit::StaticClass()))->GetComponentTransform();
	SpawnedLeftTurns += SpawnedRoom->GetLeftTurns();
	SpawnedRightTurns += SpawnedRoom->GetRightTurns();
}
