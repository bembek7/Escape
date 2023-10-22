// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Math/Vector.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APlayerBase::APlayerBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    CameraTiltTimeline = CreateDefaultSubobject<UTimelineComponent>(FName("CameraTiltTimeline"));
    CameraTiltInterp.BindUFunction(this, FName("CameraTilt"));

    SlideSpeedTimeline = CreateDefaultSubobject<UTimelineComponent>(FName("SlideSpeedTimeline"));
    SlideSpeedInterp.BindUFunction(this, FName("Sliding"));
    SlideSpeedTimeline->SetTimelineLength(3.5f);

    GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &APlayerBase::OnHit);
}

// Called when the game starts or when spawned
void APlayerBase::BeginPlay()
{
	Super::BeginPlay();
    PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerController->GetLocalPlayer()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (!InputMappingContext.IsNull())
            {
                InputSystem->AddMappingContext(InputMappingContext.LoadSynchronous(), 1);
            }
        }
    }
    MovementComponent = GetCharacterMovement();
    // setting the default movement variables
    DefaultCrouchSpeed = MovementComponent->MaxWalkSpeedCrouched;
    DefaultAirControl = MovementComponent->AirControl;
    DefaultGravityScale = MovementComponent->GravityScale;
    DefaultAcceleration = MovementComponent->MaxAcceleration;

    if (CameraTiltCurve)
    {
        CameraTiltTimeline->AddInterpFloat(CameraTiltCurve, CameraTiltInterp, FName("Degrees"), FName("Tilt"));
    }

    if (HudWidgetClass)
    {
        HudWidget = CreateWidget<UW_HUD>(PlayerController, HudWidgetClass);
        HudWidget->AddToViewport();
    }
}

// Called every frame
void APlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    AActor* Floor = MovementComponent->CurrentFloor.HitResult.GetActor();
    FVector ImpactNormal = MovementComponent->CurrentFloor.HitResult.ImpactNormal;
    if (Floor)
    {
        float RelativePitchDegrees;
        float RelativeRollDegrees;
        UKismetMathLibrary::GetSlopeDegreeAngles(Floor->GetActorRightVector(), ImpactNormal, FVector(0, 0, 1), RelativePitchDegrees, RelativeRollDegrees);
        if (UKismetMathLibrary::Abs(RelativePitchDegrees) > SlidingOffAngle || UKismetMathLibrary::Abs(RelativeRollDegrees) > SlidingOffAngle)
        {
            if(!MovementComponent->IsCrouching())CrouchSlide();
            float WorldPitchDegrees;
            float WorldRollDegrees;
            float GeneralAngle = UKismetMathLibrary::FMax(UKismetMathLibrary::Abs(RelativePitchDegrees), UKismetMathLibrary::Abs(RelativeRollDegrees));
            float SlideOffSpeedMultiplier = 0.5f;
            int SlideOffAcceleration = 4000;
            UKismetMathLibrary::GetSlopeDegreeAngles(FVector(0, 1, 0), ImpactNormal, FVector(0, 0, 1), WorldPitchDegrees, WorldRollDegrees);
            bIsSlidingOff = true;
            MovementComponent->MaxWalkSpeedCrouched = DefaultCrouchSpeed * GeneralAngle * SlideOffSpeedMultiplier;
            MovementComponent->MaxAcceleration = SlideOffAcceleration;
            MovementComponent->AddInputVector(FVector(WorldPitchDegrees, WorldRollDegrees, GeneralAngle).GetSafeNormal() * -1, true);
        }
        else
        {
            if(bIsSlidingOff)StopSlidingOff();
        }
    }
    else
    {
        if (bIsSlidingOff)StopSlidingOff();
    }
}

// Called to bind functionality to input
void APlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
    if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
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
}

bool APlayerBase::CanWallBeRunOn(const FVector& WallNormal)
{
    float WallAcceptedAngle = -0.05f;
    double WallAngle = UKismetMathLibrary::DegAcos(FVector::DotProduct(WallNormal.GetSafeNormal2D(), WallNormal));
    bool WallAngleRunnable = WallAngle < MovementComponent->GetWalkableFloorAngle();
    return WallNormal.Z >= WallAcceptedAngle && WallAngleRunnable;
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
        int LaunchFromLadderForce = 1000;
        LaunchCharacter(LadderForwardVector * LaunchFromLadderForce, false, false);
    }
    else if (bIsWallRunning)
    {
        LaunchCharacter(FindLaunchFromWallVelocity(), false, false);
        EndWallRun();
    }
    else if (bIsSlidingOff)
    {
        LaunchCharacter(FVector(0,0,MovementComponent->JumpZVelocity), false, true);
    }
    else
    {
        Jump();
    }
}

void APlayerBase::Dash()
{
    if (!bDashOnCooldown && !bIsGrappling)
    {
        bDashOnCooldown = true;
        int DashForce;
        if (MovementComponent->IsFalling()) DashForce = 2550;
        else DashForce = 2700;
        LaunchCharacter(GetControlRotation().Vector() * DashForce, true, true);
        
        GetWorldTimerManager().SetTimer(ScanDashIcon, [this]() { HudWidget->UpdateDashIconScan((DashCooldown - GetWorldTimerManager().GetTimerRemaining(ResetDashIconScan)) / DashCooldown); }, 0.01f, true);
        GetWorldTimerManager().SetTimer(ResetDashIconScan, [this]() { GetWorldTimerManager().ClearTimer(ScanDashIcon); HudWidget->UpdateDashIconScan(0.f); }, DashCooldown, false);

        GetWorldTimerManager().SetTimer(DashTimerHandle, [this]() {bDashOnCooldown = false; }, DashCooldown, false);
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
        HoldingCrouch = true;
    }
}

void APlayerBase::CrouchSlideTriggered()
{
    if (HoldingCrouch)
    {
        HoldingCrouch = false;
        if (!MovementComponent->IsCrouching())CrouchSlideStarted();
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

void APlayerBase::CrouchSlide()
{
    double PlayerSpeed = UKismetMathLibrary::VSizeXY(MovementComponent->Velocity);
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

void APlayerBase::BeginWallRun()
{
    MovementComponent->AirControl = 1;
    MovementComponent->GravityScale = 0;
    MovementComponent->SetPlaneConstraintNormal(FVector(0, 0, 1));
    bIsWallRunning = true;
    CameraTiltTimeline->Play();
    GetWorldTimerManager().SetTimer(WallRunTimer, this, &APlayerBase::UpdateWallRun, 0.1f, true);
}

void APlayerBase::EndWallRun()
{
    MovementComponent->AirControl = DefaultAirControl;
    MovementComponent->GravityScale = DefaultGravityScale;
    MovementComponent->SetPlaneConstraintNormal(FVector(0, 0, 0));
    bIsWallRunning = false;
    CameraTiltTimeline->Reverse();
    GetWorldTimerManager().ClearTimer(WallRunTimer);
}

void APlayerBase::UpdateWallRun()
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
    int TraceLength = 200;
    bool bHitWall = GetWorld()->LineTraceSingleByChannel(HitResult, GetActorLocation(), GetActorLocation() + FVector::CrossProduct(WallRunDirection, SideToTrace) * TraceLength, ECC_Visibility, FCollisionQueryParams(FName(NAME_None), false, this));
    if (bHitWall)
    {
        if (HitResult.Component->ComponentHasTag(WallRunTag) && FindRunSide(HitResult.ImpactNormal) == CurrentSide)
        {
            WallRunDirection = FindRunDirection(HitResult.ImpactNormal, CurrentSide);
            FVector WallRunForce = WallRunDirection * MovementComponent->GetMaxSpeed();
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
    int MinVelocity = 200;
    float PlayerVel = UKismetMathLibrary::Quat_UnrotateVector(FQuat(GetActorRotation()), GetVelocity()).X;
    return !bIsWallRunning && MovementComponent->IsFalling() && PlayerVel > MinVelocity && YWalkAxis >= 0 && CanWallBeRunOn(SurfaceNormal);
}

void APlayerBase::CameraTilt(float TimelineVal)
{
    int CameraTiltSide;
    switch (CurrentSide)
    {
    case Left:
        CameraTiltSide = -1;
        break;

    case Right:
        CameraTiltSide = 1;
        break;
    }
    FRotator CurrentRoation = PlayerController->GetControlRotation();
    PlayerController->SetControlRotation(FRotator(CurrentRoation.Pitch, CurrentRoation.Yaw, TimelineVal * CameraTiltSide));
}

WallRunSide APlayerBase::FindRunSide(const FVector& WallNormal)
{
    bool DotResult = FVector2D::DotProduct(FVector2D(WallNormal), FVector2D(GetActorRightVector())) > 0;
    WallRunSide SideFound;
    switch (DotResult)
    {
    case false:
        SideFound = Left;
        break;

    case true:
        SideFound = Right;
        break;
    }
    return SideFound;
}

FVector APlayerBase::FindRunDirection(const FVector& WallNormal, WallRunSide Side)
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
    float LaunchForceMultiplier = 0.55f;
    FVector AdditionalUpForce = FVector(0, 0, 150);
    return FVector((FVector::CrossProduct(WallRunDirection, SideToJumpFrom) + FVector(0, 0, 1)) * MovementComponent->JumpZVelocity * LaunchForceMultiplier + AdditionalUpForce);
}

void APlayerBase::Sliding(float Speed)
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


