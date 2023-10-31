// Fill out your copyright notice in the Description page of Project Settings.


#include "GMBase.h"
#include "Kismet/GameplayStatics.h"
#include "SaveGameBase.h"
#include "PlayerControllerBase.h"
#include "PlayerBase.h"

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

	SpawnLevel();
	BindOnDestroyedToPlayer();
}

void AGMBase::FloorStarted()
{
	APlayerControllerBase* PlayerController = Cast<APlayerControllerBase>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	CurrentFloorBeat++;
	PlayerController->ShowTimeWidget();
	CurrentTime = 0;
	GetWorldTimerManager().SetTimer(FloorTimer, [this](){ CurrentTime++; }, 0.01f, true);
}

void AGMBase::SetSavePlayedTutorial(bool Played)
{
	USaveGameBase* SaveObject = Cast<USaveGameBase>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveIndex));
	if (SaveObject)
	{
		SaveObject->bPlayedTutorial = Played;
		UGameplayStatics::SaveGameToSlot(SaveObject, SaveSlotName, SaveIndex);
	}
}

int32 AGMBase::GetSavedTime() const
{
	USaveGameBase* SaveObject = Cast<USaveGameBase>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveIndex));
	return SaveObject->GetBestTime();
}

int32 AGMBase::GetSavedFloorBeat() const
{
	USaveGameBase* SaveObject = Cast<USaveGameBase>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveIndex));
	return SaveObject->GetBestFloorCount();
}

void AGMBase::FloorCompleted()
{
	APlayerControllerBase* PlayerController = Cast<APlayerControllerBase>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	SpawnedLeftTurns = 0;
	SpawnedRightTurns = 0;
	PlayerController->HideTimeWidget();
	SaveScore();
	PlayerController->ShowFloorCompletedWidget();
	GetWorldTimerManager().ClearTimer(FloorTimer);
	SpawnRooms(NumberOfRoomsToSpawn, LastRoomExitTransform);
}

bool AGMBase::PlayedTutorial() const
{
	USaveGameBase* SaveObject = Cast<USaveGameBase>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveIndex));
	return SaveObject->bPlayedTutorial;
}

FText AGMBase::TimeToText(int32 TimeInHundredthsOfSeconds) const
{
	uint32 HundredthsOfSeconds;
	uint32 TensOfHundredthsOfSeconds;
	uint32 Seconds;
	uint32 TensOfSeconds;
	uint32 Minutes;
	uint32 TensOfMinutes;
	HundredthsOfSeconds = TimeInHundredthsOfSeconds % 10;
	TensOfHundredthsOfSeconds = TimeInHundredthsOfSeconds % 100 / 10;
	Seconds = TimeInHundredthsOfSeconds % 1000 / 100;
	TensOfSeconds = TimeInHundredthsOfSeconds % 6000 / 1000;
	Minutes = TimeInHundredthsOfSeconds % 360000 / 6000;
	TensOfMinutes = TimeInHundredthsOfSeconds / 60000;
	FString Time = FString::Printf(TEXT("%d%d:%d%d:%d%d"), TensOfMinutes, Minutes, TensOfSeconds, Seconds, TensOfHundredthsOfSeconds, HundredthsOfSeconds);
	return FText::FromString(Time);
}

FText AGMBase::GetCurrentTimeInText() const
{
	return TimeToText(CurrentTime);
}

int32 AGMBase::GetCurrentFloorBeat() const
{
	return CurrentFloorBeat;
}

void AGMBase::SaveScore()
{
	USaveGameBase* SaveObject = Cast<USaveGameBase>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveIndex));
	if (SaveObject)
	{
		SaveObject->SaveTime(CurrentTime);
		SaveObject->SaveFloorCount(CurrentFloorBeat);
		UGameplayStatics::SaveGameToSlot(SaveObject, SaveSlotName, SaveIndex);
	}
}

void AGMBase::SpawnLevel()
{
	LastRoomExitTransform = Cast<ARoom>(UGameplayStatics::GetActorOfClass(GetWorld(), ARoom::StaticClass()))->GetExitTransform();
	SpawnRooms(NumberOfRoomsToSpawn, LastRoomExitTransform);
}

void AGMBase::BindOnDestroyedToPlayer()
{
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	PlayerChar->OnDestroyed.AddDynamic(this, &AGMBase::PlayerDestroyed);
}

void AGMBase::PlayerDestroyed(AActor* DestroyedPlayer)
{
	APlayerControllerBase* PlayerController = Cast<APlayerControllerBase>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	CurrentFloorBeat = 0;
	APlayerBase* NewPlayer = GetWorld()->SpawnActor<APlayerBase>(DestroyedPlayer->GetClass(), PlayerController->GetPlayerSpawnLocation(), FRotator(0, 90, 0), FActorSpawnParameters());
	PlayerController->Possess(NewPlayer);
	NewPlayer->BindController(PlayerController);
	if (!PlayerController->bInTutorial)
	{
		ClearRooms();
		FloorStarted();
	}
	SpawnLevel();
	BindOnDestroyedToPlayer();
}

void AGMBase::ClearRooms()
{
	for (auto& Room : SpawnedRooms)
	{
		GetWorld()->DestroyActor(Room);
	}
	SpawnedRooms.Empty();
 	TArray<AActor*>GrappleTargets;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), GrappleTargetTag, GrappleTargets);
	for (auto& GrappleTarget : GrappleTargets)
	{
		GetWorld()->DestroyActor(GrappleTarget);
	}
	GrappleTargets.Empty();
	SpawnedLeftTurns = 0;
	SpawnedRightTurns = 0;
	GenerateEqualChances();
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
