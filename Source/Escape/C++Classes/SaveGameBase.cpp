// Fill out your copyright notice in the Description page of Project Settings.

#include "SaveGameBase.h"

const FString USaveGameBase::SaveSlotName = "DefaultSave";

void USaveGameBase::SaveFloorCount(const int32 NewFloorCount) noexcept
{
	BestFloorCount = FMath::Max(BestFloorCount, NewFloorCount);
}

int32 USaveGameBase::GetBestFloorCount() const noexcept
{
	return BestFloorCount;
}

void USaveGameBase::SaveTime(const int32 NewTime) noexcept
{
	BestTime = FMath::Min(BestTime, NewTime);
}

int32 USaveGameBase::GetBestTime() const noexcept
{
	return BestTime;
}