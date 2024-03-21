#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/TimelineComponent.h"
#include "TimerManager.h"
#include "GrappleLine.h"
#include "Components/SphereComponent.h"
#include "PlayerControllerBase.h"
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
	APlayerBase() noexcept;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void BindController(AController* NewController) noexcept;
	UFUNCTION(BlueprintCallable, Category = "Ladder")
	void EnteredLadder(const FVector& LadderForward) noexcept;
	UFUNCTION(BlueprintCallable, Category = "Ladder")
	void ExittedLadder() noexcept;
	UFUNCTION(BlueprintCallable, Category = "Ladder")
	void ExitLadderBoost() noexcept;
	void OnDestroyed() noexcept;

protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpuls, const FHitResult& Hit) noexcept;
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) noexcept;

	// Crouch / Slide
	UFUNCTION(BlueprintCallable, Category = "Crouch/Slide")
	void CrouchSlide() noexcept;
	UFUNCTION(BlueprintCallable, Category = "Crouch/Slide")
	void StopCrouching() noexcept;

private:
	UFUNCTION(Category = "Input Response")
	void Walk(const FInputActionValue& IAValue) noexcept;
	UFUNCTION(Category = "Input Response")
	void Look(const FInputActionValue& IAValue) noexcept;
	UFUNCTION(Category = "Input Response")
	void InputJump() noexcept;
	UFUNCTION(Category = "Input Response")
	void Dash() noexcept;
	UFUNCTION(Category = "Input Response")
	void CrouchSlideStarted() noexcept;
	UFUNCTION(Category = "Input Response")
	void CrouchSlideTriggered() noexcept;
	UFUNCTION(Category = "Input Response")
	void CrouchSlideCompleted() noexcept;
	UFUNCTION(Category = "Input Response")
	void PauseCalled() noexcept;

	// Sliding off
	void StopSlidingOff() noexcept;

	// Wall run
	bool CanWallBeRunOn(const FVector& WallNormal) const noexcept;
	bool CanWallRun(const FVector& SurfaceNormal) noexcept;
	void BeginWallRun() noexcept;
	void EndWallRun() noexcept;
	UFUNCTION()
	void UpdateWallRun() noexcept;
	UFUNCTION()
	void CameraTilt(float TimelineVal) const noexcept;
	WallRunSide FindRunSide(const FVector& WallNormal) const noexcept;
	FVector FindRunDirection(const FVector& WallNormal, WallRunSide Side) const noexcept;
	FVector FindLaunchFromWallVelocity() const noexcept;

	// Sliding
	UFUNCTION()
	void Sliding(float Speed) noexcept;

	// Grapple
	void UseGrapple() noexcept;
	void GrappleFirst() noexcept;
	void GrappleSecond() noexcept;
	UFUNCTION()
	void GrappleDragUpdate(float TimelineVal) noexcept;
	bool FindGrappleTarget() noexcept;

public:

protected:
	// Enhaced input
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	TSoftObjectPtr<UInputMappingContext> InputMappingContext;
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputAction* IAWalk;
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputAction* IAJump;
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputAction* IALook;
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputAction* IADash;
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputAction* IACrouchSlide;
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputAction* IAPause;
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputAction* IAGrapple;
	UPROPERTY(BlueprintReadOnly)
	UCharacterMovementComponent* MovementComponent;

	// Sliding off
	UPROPERTY(BlueprintReadOnly, Category = "SlidingOff")
	bool bIsSlidingOff = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlidingOff")
	float SlidingOffAngle = 15;

	// Wall run
	UPROPERTY(BlueprintReadOnly, Category = "Wall Run")
	bool bIsWallRunning = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wall Run")
	FName WallRunTag = FName("WallToRun");
	UPROPERTY(EditDefaultsOnly, Category = "Wall Run")
	UCurveFloat* CameraTiltCurve;

	// Slide
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	UCurveFloat* SlideSpeedCurve;
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	uint32 SpeedNeededToSlide = 1100;
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	uint32 SlideAdditionalSpeed = 450;

	// Dash
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashCooldown = 2.f;
	UPROPERTY(BlueprintReadOnly, Category = "Dash")
	bool bDashOnCooldown = false;

	// Grapple
	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
	TSubclassOf<AGrappleLine>GrappleLineClass;
	UPROPERTY(BlueprintReadOnly, Category = "Grapple")
	AActor* GrappleTarget;
	UPROPERTY(BlueprintReadOnly, Category = "Grapple")
	bool bIsGrappling;
	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
	uint32 GrappleImpulseMultiplier = 500;
	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
	uint32 GrappleForceMultiplier = 16000;
	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
	uint32 GrappleRange = 4250;
	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
	UCurveFloat* GrappleDragForceCurve;

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float MouseXSensitivity = 0.6f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float MouseYSensitivity = 0.6f;

	// Ladder
	UPROPERTY(BlueprintReadWrite, Category = "Ladder")
	bool bCanEnterLadder = true;
	UPROPERTY(BlueprintReadOnly, Category = "Ladder")
	bool bIsOnLadder = false;

	// Jump Pad
	UPROPERTY(EditDefaultsOnly, Category = "Jump Pad")
	FName JumpPadTag = FName("JumpPad");

	UPROPERTY(EditDefaultsOnly, Category = "Jump Pad")
	uint32 JumpPadAdditionalForce = 1000;

	// Variables to save default movement settings
	UPROPERTY(BlueprintReadOnly, Category = "SavedDefaults")
	float DefaultCrouchSpeed;
	UPROPERTY(BlueprintReadOnly, Category = "SavedDefaults")
	float DefaultAirControl;
	UPROPERTY(BlueprintReadOnly, Category = "SavedDefaults")
	float DefaultGravityScale;
	UPROPERTY(BlueprintReadOnly, Category = "SavedDefaults")
	float DefaultAcceleration;

	UPROPERTY(EditDefaultsOnly, Category = "Kill Box")
	FName KillBoxTag = FName("KillBox");

private:
	FVector LadderForwardVector;
	FVector WallRunDirection;

	TEnumAsByte<WallRunSide> CurrentSide;

	float YWalkAxis;
	float SlideSpeedDifference;
	float GrappleBeginningDistance;

	bool bCanUseGrapple = true;
	bool bHoldingCrouch = false;
	bool bGrappleCanSecondUse;
	bool bGrappleArrived;

	UPROPERTY()
	AGrappleLine* GrappleLine;
	UPROPERTY()
	USphereComponent* GrappleTargetSphereColl;

	// Timers
	FTimerHandle LadderCooldownHandle;
	FTimerHandle DashTimerHandle;
	FTimerHandle ScanDashIcon;
	FTimerHandle ResetDashIconScan;
	FTimerHandle WallRunTimer;

	// Timelines
	FOnTimelineFloat CameraTiltInterp;
	FOnTimelineFloat SlideSpeedInterp;
	FOnTimelineFloat GrappleDragSpeedInterp;

	UPROPERTY()
	UTimelineComponent* CameraTiltTimeline;
	UPROPERTY()
	UTimelineComponent* SlideSpeedTimeline;
	UPROPERTY()
	UTimelineComponent* GrappleDragTimeline;
};
