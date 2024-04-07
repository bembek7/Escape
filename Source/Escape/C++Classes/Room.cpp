#include "Room.h"

ARoom::ARoom()
{
	DefaultRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRootComponent"));
	SetRootComponent(DefaultRootComponent);
	LevelExit = CreateDefaultSubobject<ULevelExit>(TEXT("LevelExit"));
	LevelExit->SetupAttachment(RootComponent);
}

int32 ARoom::GetLeftTurns() const
{
	return LeftTurns;
}

int32 ARoom::GetRightTurns() const
{
	return RightTurns;
}

FTransform ARoom::GetExitTransform() const
{
	return LevelExit->GetComponentTransform();
}