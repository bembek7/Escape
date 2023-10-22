// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/TimelineComponent.h"
#include "TimerManager.h"
#include "W_HUD.h"

#include "PlayerBase.generated.h"

UENUM()
enum WallRunSide
{
	Left	UMETA(DisplayName = "Left"),
	Right	UMETA(DisplayName = "Right")
};

UCLASS()
class ESCAPE_API APlayerBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpuls, const FHitResult& Hit);

	UFUNCTION(Category = "Wall Run")
	bool CanWallBeRunOn(const FVector& WallNormal);

	UFUNCTION(Category = "Input Response")
	void Walk(const FInputActionValue& IAValue);

	UFUNCTION(Category = "Input Response")
	void Look(const FInputActionValue& IAValue);

	UFUNCTION(Category = "Input Response")
	void InputJump();

	UFUNCTION(Category = "Input Response")
	void Dash();

	UFUNCTION()
	void StopCrouching();

	UFUNCTION(Category = "Wall Run")
	void BeginWallRun();

	UFUNCTION(Category = "Wall Run")
	void EndWallRun();

	UFUNCTION(Category = "Wall Run")
	void UpdateWallRun();

	UFUNCTION(Category = "Wall Run")
	bool CanWallRun(const FVector& SurfaceNormal);

	UFUNCTION(Category = "Wall Run")
	void CameraTilt(float TimelineVal);

	UFUNCTION(Category = "Wall Run")
	WallRunSide FindRunSide(const FVector& WallNormal);

	UFUNCTION(Category = "Wall Run")
	FVector FindRunDirection(const FVector& WallNormal, WallRunSide Side);

	UFUNCTION(Category = "Wall Run")
	FVector FindLaunchFromWallVelocity() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	TSoftObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* IAWalk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* IAJump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* IALook;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* IADash;

	UPROPERTY()
	UCharacterMovementComponent* MovementComponent;

	UPROPERTY()
	APlayerController* PlayerController;

	UPROPERTY()
	bool bIsSlidingOff;

	UPROPERTY()
	bool bIsOnLadder;

	UPROPERTY()
	bool bIsWallRunning;

	UPROPERTY()
	FVector LadderForwardVector;

	UPROPERTY()
	float DefaultCrouchSpeed;
	
	UPROPERTY()
	float DefaultAirControl;

	UPROPERTY()
	float DefaultGravityScale;

	UPROPERTY()
	TEnumAsByte<WallRunSide> CurrentSide;

	UPROPERTY()
	FVector WallRunDirection;
	
	UPROPERTY()
	FName WallRunTag = FName("WallToRun");
	
	UPROPERTY()
	UTimelineComponent* CameraTiltTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run")
	UCurveFloat* CameraTiltCurve;

	FTimerHandle WallRunTimer;

	FOnTimelineFloat CameraTiltInterp;

	UPROPERTY()
	float MouseXSensitivity = 0.6f;

	UPROPERTY()
	float MouseYSensitivity = 0.6f;
	
	UPROPERTY()
	float YWalkAxis;

	UPROPERTY()
	bool bDashOnCooldown;

	UPROPERTY()
	bool bIsGrappling;

	UPROPERTY()
	float DashCooldown = 2.f;

	FTimerHandle DashTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UW_HUD>HudWidgetClass;

	UPROPERTY()
	UW_HUD* HudWidget;

	FTimerHandle ScanDashIcon;

	FTimerHandle ResetDashIconScan;

};
