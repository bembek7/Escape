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
	/////////////FUNCTIONS//////////////

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpuls, const FHitResult& Hit);

	// Crouch / Slide

	UFUNCTION(BlueprintCallable, Category = "Crouch/Slide")
	void CrouchSlide();

	UFUNCTION(BlueprintCallable, Category = "Crouch/Slide")
	void StopCrouching();

	/////////////VARIABLES//////////////

	// Enhaced input

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* IACrouchSlide;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCharacterMovementComponent* MovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	APlayerController* PlayerController;

	// Sliding off

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlidingOff")
	bool bIsSlidingOff;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlidingOff")
	float SlidingOffAngle = 15;

	// Wall run

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run")
	bool bIsOnLadder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run")
	bool bIsWallRunning;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run")
	FName WallRunTag = FName("WallToRun");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Run")
	UCurveFloat* CameraTiltCurve;		

	// Slide

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide")
		UCurveFloat* SlideSpeedCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide")
		int SpeedNeededToSlide = 1100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide")
		int SlideAdditionalSpeed = 450;

	// Dash

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
		float DashCooldown = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
		bool bDashOnCooldown;

	// Settings

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float MouseXSensitivity = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float MouseYSensitivity = 0.6f;	
	
	// Widgets

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UW_HUD>HudWidgetClass; // blueprint child will set the widget class

private:
	/////////////FUNCTIONS//////////////
	
	// Input responses

	UFUNCTION(Category = "Input Response")
	void Walk(const FInputActionValue& IAValue);

	UFUNCTION(Category = "Input Response")
	void Look(const FInputActionValue& IAValue);

	UFUNCTION(Category = "Input Response")
	void InputJump();

	UFUNCTION(Category = "Input Response")
	void Dash();

	UFUNCTION(Category = "Input Response")
	void CrouchSlideStarted();

	UFUNCTION(Category = "Input Response")
	void CrouchSlideTriggered();

	UFUNCTION(Category = "Input Response")
	void CrouchSlideCompleted();

	// Sliding off

	UFUNCTION(Category = "SlidingOff")
	void StopSlidingOff();

	// Wall run

	UFUNCTION(Category = "Wall Run")
	bool CanWallBeRunOn(const FVector& WallNormal);

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
	
	// Sliding
	UFUNCTION(Category = "Slide")
	void Sliding(float Speed);

	/////////////VARIABLES//////////////

	UPROPERTY()
	UW_HUD* HudWidget;

	FVector LadderForwardVector;
	FVector WallRunDirection;
	
	TEnumAsByte<WallRunSide> CurrentSide;

	// Variables to save default movement settings
	
	float DefaultCrouchSpeed;
	float DefaultAirControl;
	float DefaultGravityScale;
	float DefaultAcceleration;

	float YWalkAxis;
	float SlideSpeedDifference;
	
	bool HoldingCrouch;
	bool bIsGrappling;

	// Timers

	FTimerHandle DashTimerHandle;
	FTimerHandle ScanDashIcon;
	FTimerHandle ResetDashIconScan;
	FTimerHandle WallRunTimer;

	// Timelines

	FOnTimelineFloat CameraTiltInterp;
	FOnTimelineFloat SlideSpeedInterp;

	UPROPERTY()
	UTimelineComponent* CameraTiltTimeline;

	UPROPERTY()
	UTimelineComponent* SlideSpeedTimeline;
};
