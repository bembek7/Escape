// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveGameBase.h"

void USaveGameBase::SaveFloorCount(int NewFloorCount)
{
	BestFloorCount = FMath::Max(BestFloorCount, NewFloorCount);
}

int USaveGameBase::GetBestFloorCount()
{
	return BestFloorCount;
}

void USaveGameBase::SaveTime(int NewTime)
{
	BestTime = FMath::Max(BestTime, NewTime);
}

int USaveGameBase::GetBestTime()
{
	return BestTime;
}
