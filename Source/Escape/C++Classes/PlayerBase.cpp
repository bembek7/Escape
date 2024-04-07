// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Math/Vector.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

APlayerBase::APlayerBase()
{
	PrimaryActorTick.bCanEverTick = true;
	// Creating timelines
	CameraTiltTimeline = CreateDefaultSubobject<UTimelineComponent>(FName("CameraTiltTimeline"));
	CameraTiltInterp.BindUFunction(this, FName("CameraTilt"));

	SlideSpeedTimeline = CreateDefaultSubobject<UTimelineComponent>(FName("SlideSpeedTimeline"));
	SlideSpeedInterp.BindUFunction(this, FName("Sliding"));

	GrappleDragTimeline = CreateDefaultSubobject<UTimelineComponent>(FName("GrappleDragTimeline"));
	GrappleDragSpeedInterp.BindUFunction(this, FName("GrappleDragUpdate"));

	// Binding on hit on overlap
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &APlayerBase::OnHit);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &APlayerBase::OnBeginOverlap);
}

void APlayerBase::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetCharacterMovement();
	// Setting the default movement variables
	DefaultCrouchSpeed = MovementComponent->MaxWalkSpeedCrouched;
	DefaultAirControl = MovementComponent->AirControl;
	DefaultGravityScale = MovementComponent->GravityScale;
	DefaultAcceleration = MovementComponent->MaxAcceleration;

	// Adding interps to timelines
	if (CameraTiltCurve)
	{
		CameraTiltTimeline->AddInterpFloat(CameraTiltCurve, CameraTiltInterp, FName("Degrees"), FName("Tilt"));
	}

	if (SlideSpeedCurve)
	{
		SlideSpeedTimeline->AddInterpFloat(SlideSpeedCurve, SlideSpeedInterp, FName("Speed"), FName("SpeedTrack"));
	}

	if (GrappleDragForceCurve)
	{
		GrappleDragTimeline->AddInterpFloat(GrappleDragForceCurve, GrappleDragSpeedInterp, FName("Speed"), FName("GrappleDragSpeed"));
	}
}

void APlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Slide off check
	AActor* const Floor = MovementComponent->CurrentFloor.HitResult.GetActor();
	const FVector ImpactNormal = MovementComponent->CurrentFloor.HitResult.ImpactNormal;
	if (Floor) // Checking if player should slide off the floor he is standing on or if he should stop sliding
	{
		float RelativePitchDegrees;
		float RelativeRollDegrees;
		UKismetMathLibrary::GetSlopeDegreeAngles(Floor->GetActorRightVector(), ImpactNormal, FVector(0, 0, 1), RelativePitchDegrees, RelativeRollDegrees);
		if (UKismetMathLibrary::Abs(RelativePitchDegrees) > SlidingOffAngle || UKismetMathLibrary::Abs(RelativeRollDegrees) > SlidingOffAngle)
		{
			if (!MovementComponent->IsCrouching())
			{
				CrouchSlide();
			}
			float WorldPitchDegrees;
			float WorldRollDegrees;
			const float GeneralAngle = UKismetMathLibrary::FMax(UKismetMathLibrary::Abs(RelativePitchDegrees), UKismetMathLibrary::Abs(RelativeRollDegrees));
			const float SlideOffSpeedMultiplier = 0.5f;
			const uint32 SlideOffAcceleration = 4000;
			UKismetMathLibrary::GetSlopeDegreeAngles(FVector(0, 1, 0), ImpactNormal, FVector(0, 0, 1), WorldPitchDegrees, WorldRollDegrees);
			bIsSlidingOff = true;
			MovementComponent->MaxWalkSpeedCrouched = DefaultCrouchSpeed * GeneralAngle * SlideOffSpeedMultiplier;
			MovementComponent->MaxAcceleration = SlideOffAcceleration;
			MovementComponent->AddInputVector(FVector(WorldPitchDegrees, WorldRollDegrees, GeneralAngle).GetSafeNormal() * -1, true);
		}
		else
		{
			if (bIsSlidingOff)
			{
				StopSlidingOff();
			}
		}
	}
	else
	{
		if (bIsSlidingOff)
		{
			StopSlidingOff();
		}
	}
}

void APlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (GetController())
	{
		BindController(GetController());
	}
	if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Binding inputs
		if (IAWalk)
		{
			PlayerEnhancedInputComponent->BindAction(IAWalk, ETriggerEvent::Triggered, this, &APlayerBase::Walk);
		}

		if (IAJump)
		{
			PlayerEnhancedInputComponent->BindAction(IAJump, ETriggerEvent::Started, this, &APlayerBase::InputJump);
		}

		if (IALook)
		{
			PlayerEnhancedInputComponent->BindAction(IALook, ETriggerEvent::Triggered, this, &APlayerBase::Look);
		}

		if (IADash)
		{
			PlayerEnhancedInputComponent->BindAction(IADash, ETriggerEvent::Started, this, &APlayerBase::Dash);
		}

		if (IACrouchSlide)
		{
			PlayerEnhancedInputComponent->BindAction(IACrouchSlide, ETriggerEvent::Started, this, &APlayerBase::CrouchSlideStarted);
			PlayerEnhancedInputComponent->BindAction(IACrouchSlide, ETriggerEvent::Triggered, this, &APlayerBase::CrouchSlideTriggered);
			PlayerEnhancedInputComponent->BindAction(IACrouchSlide, ETriggerEvent::Completed, this, &APlayerBase::CrouchSlideCompleted);
		}

		if (IAGrapple)
		{
			PlayerEnhancedInputComponent->BindAction(IAGrapple, ETriggerEvent::Started, this, &APlayerBase::UseGrapple);
		}

		if (IAPause)
		{
			PlayerEnhancedInputComponent->BindAction(IAPause, ETriggerEvent::Started, this, &APlayerBase::PauseCalled);
		}
	}
}

void APlayerBase::BindController(AController* NewController)
{
	if (APlayerController* const PlayerController = Cast<APlayerControllerBase>(NewController))
	{
		// Adding Mapping Context
		if (ULocalPlayer* const LocalPlayer = Cast<ULocalPlayer>(PlayerController->GetLocalPlayer()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (!InputMappingContext.IsNull())
				{
					InputSystem->AddMappingContext(InputMappingContext.LoadSynchronous(), 1);
				}
			}
		}
	}
}

void APlayerBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpuls, const FHitResult& Hit)
{
	if (OtherComp->ComponentHasTag(WallRunTag) && CanWallRun(Hit.ImpactNormal))
	{
		// init wall run
		CurrentSide = FindRunSide(Hit.ImpactNormal);
		WallRunDirection = FindRunDirection(Hit.ImpactNormal, CurrentSide);
		BeginWallRun();
	}
	if (OtherComp->ComponentHasTag(KillBoxTag))
	{
		if (APlayerControllerBase* const PlayerController = Cast<APlayerControllerBase>(GetController()))
		{
			PlayerController->ShowDeathWidget();
		}
	}
}

void APlayerBase::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherComp->ComponentHasTag(JumpPadTag))
	{
		const float ForceMultiplier = 0.6f;
		const float LaunchForce = FMath::Abs(MovementComponent->Velocity.Z) * ForceMultiplier + JumpPadAdditionalForce;
		LaunchCharacter(FVector(0, 0, LaunchForce), false, true);
	}
}

bool APlayerBase::CanWallBeRunOn(const FVector& WallNormal) const
{
	const float WallAcceptedAngle = -0.05f;
	const double WallAngle = UKismetMathLibrary::DegAcos(FVector::DotProduct(WallNormal.GetSafeNormal2D(), WallNormal));
	const bool WallAngleRunnable = WallAngle < MovementComponent->GetWalkableFloorAngle();
	return WallNormal.Z >= WallAcceptedAngle && WallAngleRunnable;
}

void APlayerBase::EnteredLadder(const FVector& LadderForward)
{
	if (bCanEnterLadder)
	{
		bCanEnterLadder = false;
		LadderForwardVector = LadderForward;
		bIsOnLadder = true;
		MovementComponent->SetMovementMode(EMovementMode::MOVE_Flying);
		MovementComponent->StopMovementImmediately();
	}
}

void APlayerBase::ExittedLadder()
{
	bIsOnLadder = false;
	if (!bIsGrappling)
	{
		MovementComponent->SetMovementMode(EMovementMode::MOVE_Walking);
	}
	const float LadderCooldown = 0.2f;
	GetWorldTimerManager().SetTimer(LadderCooldownHandle, [this]() { bCanEnterLadder = true; }, LadderCooldown, false);
}

void APlayerBase::ExitLadderBoost()
{
	const uint32 LaunchForce = 50;
	LaunchCharacter(GetActorUpVector() * LaunchForce, false, false);
}

void APlayerBase::OnDestroyed()
{
	if (ScanDashIcon.IsValid())
	{
		GetWorldTimerManager().ClearTimer(ScanDashIcon);
	}
}

void APlayerBase::Walk(const FInputActionValue& IAValue)
{
	const FVector2D MoveVector = IAValue.Get<FVector2D>();
	float XAxis = MoveVector.X;
	YWalkAxis = MoveVector.Y;
	if (bIsOnLadder)
	{
		if (XAxis != 0)
		{
			if (FVector::DotProduct(LadderForwardVector, GetActorRightVector() * XAxis) < 0)
			{
				AddMovementInput(GetActorUpVector(), 1);
			}
			else
			{
				AddMovementInput(GetActorUpVector(), -1);
			}
		}
		if (YWalkAxis != 0)
		{
			if (FVector::DotProduct(LadderForwardVector, GetActorForwardVector() * YWalkAxis) < 0)
			{
				AddMovementInput(GetActorUpVector(), 1);
			}
			else
			{
				AddMovementInput(GetActorUpVector(), -1);
			}
		}
	}
	else
	{
		if (bIsSlidingOff) // player has restricted move while sliding off
		{
			XAxis /= 10;
			YWalkAxis = 0;
		}
		AddMovementInput(GetActorRightVector(), XAxis); // moving in X-axis
		AddMovementInput(GetActorForwardVector(), YWalkAxis); // moving in Y-axis
	}
}

void APlayerBase::Look(const FInputActionValue& IAValue)
{
	const FVector2D LookVector = IAValue.Get<FVector2D>();
	AddControllerYawInput(LookVector.X * MouseXSensitivity);
	AddControllerPitchInput(LookVector.Y * MouseYSensitivity * -1);
}

void APlayerBase::InputJump()
{
	StopCrouching();
	if (bIsOnLadder)
	{
		uint32 LaunchFromLadderForce = 1000;
		LaunchCharacter(LadderForwardVector * LaunchFromLadderForce, false, false);
		ExittedLadder();
	}
	else if (bIsWallRunning)
	{
		LaunchCharacter(FindLaunchFromWallVelocity(), false, false);
		EndWallRun();
	}
	else if (bIsSlidingOff)
	{
		LaunchCharacter(FVector(0, 0, MovementComponent->JumpZVelocity), false, true);
	}
	else
	{
		Jump();
	}
}

void APlayerBase::Dash()// Dash ability
{
	if (!bDashOnCooldown && !bIsGrappling)
	{
		bDashOnCooldown = true;
		uint32 DashForce;
		if (MovementComponent->IsFalling())
		{
			DashForce = 2550; // Dash is stronger if player is not in air
		}
		else
		{
			DashForce = 2700;
		}
		LaunchCharacter(GetControlRotation().Vector() * DashForce, true, true);
		APlayerControllerBase* const PlayerController = Cast<APlayerControllerBase>(GetController());
		if (PlayerController)
		{
			GetWorldTimerManager().SetTimer(ScanDashIcon, [this, PlayerController]() { PlayerController->UpdateDashIconScanHudWidget((DashCooldown - GetWorldTimerManager().GetTimerRemaining(ResetDashIconScan)) / DashCooldown); }, 0.01f, true);
			GetWorldTimerManager().SetTimer(ResetDashIconScan, [this, PlayerController]() { GetWorldTimerManager().ClearTimer(ScanDashIcon); PlayerController->UpdateDashIconScanHudWidget(0.f); }, DashCooldown, false);
			GetWorldTimerManager().SetTimer(DashTimerHandle, [this]() {bDashOnCooldown = false; }, DashCooldown, false);
		}
	}
}

void APlayerBase::CrouchSlideStarted()
{
	if (MovementComponent->IsMovingOnGround())
	{
		CrouchSlide();
	}
	else
	{
		bHoldingCrouch = true;
	}
}

void APlayerBase::CrouchSlideTriggered()
{
	if (bHoldingCrouch)
	{
		bHoldingCrouch = false;
		if (!MovementComponent->IsCrouching())
		{
			CrouchSlideStarted();
		}
	}
}

void APlayerBase::CrouchSlideCompleted()
{
	if (!bIsSlidingOff)
	{
		UnCrouch();
		MovementComponent->MaxWalkSpeedCrouched = DefaultCrouchSpeed;
	}
}

void APlayerBase::PauseCalled()
{
	if (APlayerControllerBase* const PlayerController = Cast<APlayerControllerBase>(GetController()))
	{
		PlayerController->PauseUnpause();
	}
}

void APlayerBase::CrouchSlide() // If player is fast enough we initiate slide if not we initiate normal crouch
{
	const double PlayerSpeed = UKismetMathLibrary::VSizeXY(MovementComponent->Velocity);
	Crouch();
	if (PlayerSpeed >= SpeedNeededToSlide)
	{
		MovementComponent->MaxWalkSpeedCrouched = PlayerSpeed + SlideAdditionalSpeed;
		SlideSpeedDifference = MovementComponent->MaxWalkSpeedCrouched - DefaultCrouchSpeed;
		MovementComponent->AddImpulse(MovementComponent->Velocity.GetSafeNormal() * PlayerSpeed);
		SlideSpeedTimeline->PlayFromStart();
	}
	else
	{
		MovementComponent->MaxWalkSpeedCrouched = DefaultCrouchSpeed;
	}
}

void APlayerBase::StopCrouching()
{
	UnCrouch();
	MovementComponent->MaxWalkSpeedCrouched = DefaultCrouchSpeed;
}

void APlayerBase::StopSlidingOff()
{
	MovementComponent->MaxWalkSpeedCrouched = DefaultCrouchSpeed;
	MovementComponent->MaxAcceleration = DefaultAcceleration;
	bIsSlidingOff = false;
	StopCrouching();
}

void APlayerBase::BeginWallRun() // Setting the values for wall running or movement and tilting camera
{
	MovementComponent->AirControl = 1;
	MovementComponent->GravityScale = 0;
	MovementComponent->SetPlaneConstraintNormal(FVector(0, 0, 1));
	bIsWallRunning = true;
	CameraTiltTimeline->Play();
	GetWorldTimerManager().SetTimer(WallRunTimer, this, &APlayerBase::UpdateWallRun, 0.1f, true);
}

void APlayerBase::EndWallRun() // Setting the default values or movement and tilting camera back
{
	MovementComponent->AirControl = DefaultAirControl;
	MovementComponent->GravityScale = DefaultGravityScale;
	MovementComponent->SetPlaneConstraintNormal(FVector(0, 0, 0));
	bIsWallRunning = false;
	CameraTiltTimeline->Reverse();
	GetWorldTimerManager().ClearTimer(WallRunTimer);
}

void APlayerBase::UpdateWallRun() // Wall run function called every tick when wallrunning setting velocity or ending the wall run
{
	FHitResult HitResult;
	FVector SideToTrace;
	switch (CurrentSide)
	{
	case Left:
		SideToTrace = FVector(0, 0, -1);
		break;

	case Right:
		SideToTrace = FVector(0, 0, 1);
		break;
	}
	const uint32 TraceLength = 200;
	const FVector TraceEnd = GetActorLocation() + FVector::CrossProduct(WallRunDirection, SideToTrace) * TraceLength;
	const bool bHitWall = GetWorld()->LineTraceSingleByChannel(HitResult, GetActorLocation(), TraceEnd, ECC_Visibility, FCollisionQueryParams(FName(NAME_None), false, this));
	if (bHitWall)
	{
		if (HitResult.Component->ComponentHasTag(WallRunTag) && FindRunSide(HitResult.ImpactNormal) == CurrentSide)
		{
			WallRunDirection = FindRunDirection(HitResult.ImpactNormal, CurrentSide);
			const FVector WallRunForce = WallRunDirection * MovementComponent->GetMaxSpeed();
			MovementComponent->Velocity = FVector(WallRunForce.X, WallRunForce.Y, 0);
		}
		else
		{
			EndWallRun();
		}
	}
	else
	{
		EndWallRun();
	}
}

bool APlayerBase::CanWallRun(const FVector& SurfaceNormal)
{
	const uint32 MinVelocity = 200;
	const float PlayerVelocity = UKismetMathLibrary::Quat_UnrotateVector(FQuat(GetActorRotation()), GetVelocity()).X;
	return !bIsWallRunning && MovementComponent->IsFalling() && PlayerVelocity > MinVelocity && YWalkAxis >= 0 && CanWallBeRunOn(SurfaceNormal);
}

void APlayerBase::CameraTilt(float TimelineVal) const // When wallrunning camera tilts a little bit
{
	int32 CameraTiltSide;
	switch (CurrentSide)
	{
	case Left:
		CameraTiltSide = -1;
		break;

	case Right:
		CameraTiltSide = 1;
		break;
	}
	if (APlayerControllerBase* const PlayerController = Cast<APlayerControllerBase>(GetController()))
	{
		const FRotator CurrentRoation = PlayerController->GetControlRotation();
		PlayerController->SetControlRotation(FRotator(CurrentRoation.Pitch, CurrentRoation.Yaw, TimelineVal * CameraTiltSide));
	}
}

WallRunSide APlayerBase::FindRunSide(const FVector& WallNormal) const
{
	const double DotResult = FVector2D::DotProduct(FVector2D(WallNormal), FVector2D(GetActorRightVector()));
	WallRunSide SideFound;
	if (DotResult < 0)
	{
		SideFound = Left;
	}
	else
	{
		SideFound = Right;
	}
	return SideFound;
}

FVector APlayerBase::FindRunDirection(const FVector& WallNormal, WallRunSide Side) const
{
	FVector RunSide;
	switch (Side)
	{
	case Left:
		RunSide = FVector(0, 0, -1);
		break;

	case Right:
		RunSide = FVector(0, 0, 1);
		break;

	default:
		break;
	}
	return FVector::CrossProduct(WallNormal, RunSide);
}

FVector APlayerBase::FindLaunchFromWallVelocity() const
{
	FVector SideToJumpFrom;
	switch (CurrentSide)
	{
	case Left:
		SideToJumpFrom = FVector(0, 0, 1);
		break;

	case Right:
		SideToJumpFrom = FVector(0, 0, -1);
		break;
	}
	const float LaunchForceMultiplier = 0.55f;
	const FVector AdditionalUpForce = FVector(0, 0, 200);
	return FVector((FVector::CrossProduct(WallRunDirection, SideToJumpFrom) + FVector(0, 0, 1)) * MovementComponent->JumpZVelocity * LaunchForceMultiplier + AdditionalUpForce);
}

void APlayerBase::Sliding(float Speed) // Slide function called every tick when sliding // should add some left-right input constraints
{
	MovementComponent->MaxWalkSpeedCrouched -= Speed * SlideSpeedDifference;
	SlideSpeedDifference = MovementComponent->MaxWalkSpeedCrouched - DefaultCrouchSpeed;
	if (MovementComponent->IsCrouching())
	{
		if (MovementComponent->MaxWalkSpeedCrouched <= DefaultCrouchSpeed + 50)
		{
			MovementComponent->MaxWalkSpeedCrouched = DefaultCrouchSpeed;
			Crouch();
			SlideSpeedTimeline->Stop();
		}
	}
	else
	{
		SlideSpeedTimeline->Stop();
	}
}

void APlayerBase::UseGrapple()
{
	if (!bIsGrappling)
	{
		if (bCanUseGrapple && !bGrappleCanSecondUse && FindGrappleTarget())
		{
			GrappleFirst();
		}
	}
	else
	{
		GrappleSecond();
	}
}

void APlayerBase::GrappleFirst()
{
	GrappleLine = GetWorld()->SpawnActor<AGrappleLine>(GrappleLineClass, GetActorTransform(), FActorSpawnParameters());
	GrappleLine->GrappleOn(GrappleTarget->GetActorLocation());
	bGrappleCanSecondUse = true;
	GrappleTargetSphereColl = Cast<USphereComponent>(GrappleTarget->GetComponentByClass(USphereComponent::StaticClass()));
	bGrappleArrived = false;
	bIsGrappling = true;
	bCanUseGrapple = true;
	MovementComponent->SetMovementMode(EMovementMode::MOVE_Flying);
	MovementComponent->StopMovementImmediately();
	if (APlayerControllerBase* const PlayerController = Cast<APlayerControllerBase>(GetController()))
	{
		PlayerController->SetIgnoreMoveInput(true);
	}
	GrappleBeginningDistance = UKismetMathLibrary::Vector_Distance(GetActorLocation(), GrappleTarget->GetActorLocation());
	GrappleDragTimeline->PlayFromStart();
}

void APlayerBase::GrappleSecond()
{
	if (APlayerControllerBase* const PlayerController = Cast<APlayerControllerBase>(GetController()))
	{
		PlayerController->SetIgnoreMoveInput(false);
	}
	GrappleLine->GrappleOff();
	bIsGrappling = false;
	bGrappleCanSecondUse = false;
	GrappleDragTimeline->Stop();
	MovementComponent->SetMovementMode(MOVE_Walking);
	FVector ZImpulseToAddOnEnd;
	if (bGrappleArrived)
	{
		ZImpulseToAddOnEnd.Z = 225;
	}
	else
	{
		ZImpulseToAddOnEnd.Z = 185;
	}
	const FVector GrappleForce = (MovementComponent->Velocity.GetSafeNormal() + ZImpulseToAddOnEnd) * GrappleImpulseMultiplier;
	MovementComponent->AddImpulse(GrappleForce);
}

void APlayerBase::GrappleDragUpdate(float TimelineVal)
{
	const FVector GrappleDirection = (GrappleTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	const FVector GrappleForce = GrappleDirection * GrappleForceMultiplier * MovementComponent->Mass * TimelineVal;
	MovementComponent->AddForce(GrappleForce);
	if (GrappleTargetSphereColl->IsOverlappingActor(this))
	{
		bGrappleArrived = true;
		GrappleSecond();
	}
}

bool APlayerBase::FindGrappleTarget()
{
	GrappleTarget = nullptr;
	const float TraceRadius = GrappleRange;
	TArray<FHitResult> TraceResults;
	TArray<TEnumAsByte<EObjectTypeQuery>>GrappleTargetType;
	const EObjectTypeQuery GrappleType = UCollisionProfile::Get()->ConvertToObjectType(ECC_GameTraceChannel1);  // grapple object type
	if (GrappleType)
	{
		GrappleTargetType.Add(GrappleType);
	}
	UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), GetActorLocation(), GetActorLocation(), TraceRadius, GrappleTargetType, false, TArray<AActor*>(), EDrawDebugTrace::None, TraceResults, false);
	float Difference = 999999; // Difference in distance between grapple target and player's cursor
	const FVector PlayerLooks = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetActorForwardVector();
	for (const auto& Hit : TraceResults)
	{
		if (AActor* const TargetFound = Hit.GetActor())
		{
			const bool bTargetOnScreen = TargetFound->WasRecentlyRendered();
			const FVector TargetDirection = FVector(TargetFound->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			const float DistanceTargetToCursor = UKismetMathLibrary::Vector_Distance(TargetDirection, PlayerLooks);
			const bool bTargetCloserToCursor = DistanceTargetToCursor < Difference;
			if (bTargetOnScreen && bTargetCloserToCursor)
			{
				Difference = DistanceTargetToCursor;
				GrappleTarget = TargetFound;
			}
		}
	}
	return UKismetSystemLibrary::IsValid(GrappleTarget);
}