// Fill out your copyright notice in the Description page of Project Settings.

#include "GMBase.h"
#include "Kismet/GameplayStatics.h"
#include "SaveGameBase.h"
#include "PlayerControllerBase.h"
#include "PlayerBase.h"

AGMBase::AGMBase() noexcept
{
	if (!UGameplayStatics::DoesSaveGameExist(USaveGameBase::SaveSlotName, USaveGameBase::SaveIndex))
	{
		USaveGame* SaveObject = UGameplayStatics::CreateSaveGameObject(USaveGameBase::StaticClass());
		UGameplayStatics::SaveGameToSlot(SaveObject, USaveGameBase::SaveSlotName, USaveGameBase::SaveIndex);
	}
}

void AGMBase::BeginPlay()
{
	Super::BeginPlay();

	RoomsChancesOfSpawning.Init(0, RoomsClasses.Num());
	GenerateEqualChances();
}

void AGMBase::FloorStarted() noexcept
{
	APlayerControllerBase* PlayerController = Cast<APlayerControllerBase>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PlayerController)
	{
		PlayerController->ShowTimeWidget();
	}
	CurrentFloorBeat++;
	CurrentTime = 0;
	GetWorldTimerManager().SetTimer(FloorTimer, [this]() { CurrentTime++; }, 0.01f, true);
}

void AGMBase::SetSavePlayedTutorial(bool Played) noexcept
{
	USaveGameBase* SaveObject = Cast<USaveGameBase>(UGameplayStatics::LoadGameFromSlot(USaveGameBase::SaveSlotName, USaveGameBase::SaveIndex));
	if (SaveObject)
	{
		SaveObject->bPlayedTutorial = Played;
		UGameplayStatics::SaveGameToSlot(SaveObject, USaveGameBase::SaveSlotName, USaveGameBase::SaveIndex);
	}
}

int32 AGMBase::GetSavedTime() const noexcept
{
	const USaveGameBase* SaveObject = Cast<USaveGameBase>(UGameplayStatics::LoadGameFromSlot(USaveGameBase::SaveSlotName, USaveGameBase::SaveIndex));
	if (SaveObject)
	{
		return SaveObject->GetBestTime();
	}
	return 0;
}

int32 AGMBase::GetSavedFloorBeat() const noexcept
{
	const USaveGameBase* SaveObject = Cast<USaveGameBase>(UGameplayStatics::LoadGameFromSlot(USaveGameBase::SaveSlotName, USaveGameBase::SaveIndex)); 
	if (SaveObject)
	{
		return SaveObject->GetBestFloorCount();
	}
	return 0;
}

void AGMBase::FloorCompleted() noexcept
{
	SpawnedLeftTurns = 0;
	SpawnedRightTurns = 0;
	SaveScore();
	APlayerControllerBase* PlayerController = Cast<APlayerControllerBase>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PlayerController)
	{
		PlayerController->HideTimeWidget();
		PlayerController->ShowFloorCompletedWidget();
	}
	GetWorldTimerManager().ClearTimer(FloorTimer);
	SpawnRooms(NumberOfRoomsToSpawn, LastRoomExitTransform);
}

FText AGMBase::TimeToText(int32 TimeInHundredthsOfSeconds) const noexcept
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
	const FString Time = FString::Printf(TEXT("%d%d:%d%d:%d%d"), TensOfMinutes, Minutes, TensOfSeconds, Seconds, TensOfHundredthsOfSeconds, HundredthsOfSeconds);
	return FText::FromString(Time);
}

FText AGMBase::GetCurrentTimeInText() const noexcept
{
	return TimeToText(CurrentTime);
}

int32 AGMBase::GetCurrentFloorBeat() const noexcept
{
	return CurrentFloorBeat;
}

void AGMBase::GameStarted() noexcept
{
	SpawnLevel();
	BindOnDestroyedToPlayer();
}

void AGMBase::SaveScore() const noexcept
{
	USaveGameBase* SaveObject = Cast<USaveGameBase>(UGameplayStatics::LoadGameFromSlot(USaveGameBase::SaveSlotName, USaveGameBase::SaveIndex));
	if (SaveObject)
	{
		SaveObject->SaveTime(CurrentTime);
		SaveObject->SaveFloorCount(CurrentFloorBeat);
		UGameplayStatics::SaveGameToSlot(SaveObject, USaveGameBase::SaveSlotName, USaveGameBase::SaveIndex);
	}
}

void AGMBase::SpawnLevel() noexcept
{
	ARoom* LastRoom = Cast<ARoom>(UGameplayStatics::GetActorOfClass(GetWorld(), ARoom::StaticClass()));
	if(LastRoom)
	{
		LastRoomExitTransform = LastRoom->GetExitTransform();
		SpawnRooms(NumberOfRoomsToSpawn, LastRoomExitTransform);
	}
}

void AGMBase::BindOnDestroyedToPlayer() const noexcept
{
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerChar)
	{
		PlayerChar->OnDestroyed.AddDynamic(this, &AGMBase::PlayerDestroyed);
	}
}

void AGMBase::PlayerDestroyed(AActor* DestroyedPlayer) noexcept
{
	APlayerControllerBase* PlayerController = Cast<APlayerControllerBase>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PlayerController)
	{
		PlayerController->UpdateDashIconScanHudWidget(0.f);
		APlayerBase* DestroyedPlayerCasted = Cast<APlayerBase>(DestroyedPlayer);
		if (DestroyedPlayerCasted)
		{
			DestroyedPlayerCasted->OnDestroyed();
		}
		CurrentFloorBeat = 0;
		APlayerBase* NewPlayer = GetWorld()->SpawnActor<APlayerBase>(DestroyedPlayer->GetClass(), PlayerController->GetPlayerSpawnLocation(), FRotator(0, 90, 0), FActorSpawnParameters());
		PlayerController->Possess(NewPlayer);
		NewPlayer->BindController(PlayerController);
		if (!PlayerController->bInTutorial)
		{
			PlayerController->HideTimeWidget();
			ClearRooms();
			FloorStarted();
			SpawnLevel();
		}
		BindOnDestroyedToPlayer();
	}
}

void AGMBase::ClearRooms() noexcept
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

TSubclassOf<ARoom> AGMBase::GetRandomRoomClass() noexcept
{
	const uint32 RandomIndex = GetRandomRoomIndex();
	const TSubclassOf<ARoom> RandomClass = RoomsClasses[RandomIndex];
	if (RandomClass.GetDefaultObject()->GetLeftTurns() + SpawnedLeftTurns >= 4 || RandomClass.GetDefaultObject()->GetRightTurns() + SpawnedRightTurns >= 4)
	{
		GetRandomRoomClass();
	}
	return RandomClass;
}

uint32 AGMBase::GetRandomRoomIndex() noexcept
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
	RandomizedClassIndex = AllRoomsIndexesByChances[FMath::RandRange(0, AllRoomsIndexesByChances.Num() - 1)];
	UpdateChances(RandomizedClassIndex);
	return RandomizedClassIndex;
}

void AGMBase::UpdateChances(const uint32 ChosenIndex) noexcept
{
	const uint32 ChancesOfChosenIndex = RoomsChancesOfSpawning[ChosenIndex];
	// we will set chances of index we got to 0 so we are distributing the chances of it to other classes chances
	const uint32 DistributionOnOther = ChancesOfChosenIndex / (RoomsChancesOfSpawning.Num() - 1);
	uint32 DistributionRest = ChancesOfChosenIndex % (RoomsChancesOfSpawning.Num() - 1);
	uint32 LoopIndex = 0;
	for (auto& Chance : RoomsChancesOfSpawning)
	{
		if (LoopIndex != ChosenIndex)
		{
			Chance += DistributionOnOther;
			Chance += DistributionRest;
			if (DistributionRest > 0)
			{
				DistributionRest--;
			}
		}
		else
		{
			Chance = 0;
		}
		LoopIndex++;
	}
}

void AGMBase::GenerateEqualChances() noexcept
{
	for (auto& chance : RoomsChancesOfSpawning)
	{
		chance = 100 / RoomsChancesOfSpawning.Num();
	}
	RoomsChancesOfSpawning.Last() += 100 % RoomsChancesOfSpawning.Num();
}

void AGMBase::SpawnRooms(const uint32 RoomsToSpawn, const FTransform& LastExitTransform) noexcept
{
	if (RoomsToSpawn > 0)
	{
		SpawnRoom(GetRandomRoomClass(), LastExitTransform);
		SpawnRooms(RoomsToSpawn - 1, LastRoomExitTransform);
	}
	else
	{
		SpawnRoom(FinalRoomClass, LastExitTransform);
	}
}

void AGMBase::SpawnRoom(const TSubclassOf<ARoom>& RoomClass, const FTransform& SpawnTransform) noexcept
{
	ARoom* SpawnedRoom = GetWorld()->SpawnActor<ARoom>(RoomClass.Get(), SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator(), FActorSpawnParameters());
	SpawnedRooms.Add(SpawnedRoom);
	LastRoomExitTransform = SpawnedRoom->GetExitTransform();
	SpawnedLeftTurns += SpawnedRoom->GetLeftTurns();
	SpawnedRightTurns += SpawnedRoom->GetRightTurns();
}