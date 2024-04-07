#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelExit.h"
#include "Room.generated.h"

UCLASS()
class ESCAPE_API ARoom : public AActor
{
	GENERATED_BODY()

public:
	ARoom();

	UFUNCTION(BlueprintCallable)
	int32 GetLeftTurns() const;

	UFUNCTION(BlueprintCallable)
	int32 GetRightTurns() const;

	FTransform GetExitTransform() const;

protected:

private:

public:

protected:
	UPROPERTY(EditDefaultsOnly)
	uint32 LeftTurns;

	UPROPERTY(EditDefaultsOnly)
	uint32 RightTurns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ULevelExit* LevelExit;
private:
	UPROPERTY()
	USceneComponent* DefaultRootComponent;
};
