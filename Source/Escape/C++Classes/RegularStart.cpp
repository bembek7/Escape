// Fill out your copyright notice in the Description page of Project Settings.


#include "RegularStart.h"

// Sets default values
ARegularStart::ARegularStart()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARegularStart::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARegularStart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

