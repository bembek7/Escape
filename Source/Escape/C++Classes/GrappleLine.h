// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../../../../../../../Program Files/Epic Games/UE_5.1/Engine/Plugins/Runtime/CableComponent/Source/CableComponent/Classes/CableComponent.h"
#include "GrappleLine.generated.h"

UCLASS()
class ESCAPE_API AGrappleLine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrappleLine();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void GrappleOn(const FVector& Target);
	UFUNCTION(BlueprintCallable)
	void GrappleOff();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCableComponent* GrappleLine;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DevelopingSpeed = 4.5f;
private:
	UPROPERTY()
	FVector GrappleTarget;
	UPROPERTY()
	bool bDeveloping;
	UPROPERTY()
	FTimerHandle LineDevelopingTimer;
};
