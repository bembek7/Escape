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

    GrappleDragTimeline = CreateDefaultSubobject<UTimelineComponent>(FName("GrappleDragTimeline"));
    GrappleDragSpeedInterp.BindUFunction(this, FName("GrappleDragUpdate"));

    GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &APlayerBase::OnHit);
    GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &APlayerBase::OnBeginOverlap);
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

    if (SlideSpeedCurve)
    {
        SlideSpeedTimeline->AddInterpFloat(SlideSpeedCurve, SlideSpeedInterp, FName("Speed"), FName("SpeedTrack"));
    }

    if (GrappleDragForceCurve)
    {
        GrappleDragTimeline->AddInterpFloat(GrappleDragForceCurve, GrappleDragSpeedInterp, FName("Speed"), FName("GrappleDragSpeed"));
    }

    CreateWidgets();
    HudWidget->AddToPlayerScreen();
    // ShowWidgetToFocus(MainMenu);
}

// Called every frame
void APlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    AActor* Floor = MovementComponent->CurrentFloor.HitResult.GetActor();
    FVector ImpactNormal = MovementComponent->CurrentFloor.HitResult.ImpactNormal;
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

// Called to bind functionality to input
void APlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
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
            PlayerEnhancedInputComponent->BindAction(IAPause, ETriggerEvent::Started, this, &APlayerBase::PauseUnpause);
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

void APlayerBase::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherComp->ComponentHasTag(JumpPadTag))
    {
        float LaunchForce;
        float ForceMultiplier = 0.6f;
        LaunchForce = FMath::Abs(MovementComponent->Velocity.Z) * ForceMultiplier + JumpPadAdditionalForce;
        LaunchCharacter(FVector(0, 0, LaunchForce), false, true); 
    }
}

bool APlayerBase::CanWallBeRunOn(const FVector& WallNormal)
{
    float WallAcceptedAngle = -0.05f;
    double WallAngle = UKismetMathLibrary::DegAcos(FVector::DotProduct(WallNormal.GetSafeNormal2D(), WallNormal));
    bool WallAngleRunnable = WallAngle < MovementComponent->GetWalkableFloorAngle();
    return WallNormal.Z >= WallAcceptedAngle && WallAngleRunnable;
}

void APlayerBase::CreateWidgets()
{
    if (HudWidgetClass)
    {
        HudWidget = CreateWidget<UW_HUD>(PlayerController, HudWidgetClass);
    }

    if (PauseWidgetClass)
    {
        PauseWidget = CreateWidget<UUserWidget>(PlayerController, PauseWidgetClass);
    }

    if (MainMenuClass)
    {
        MainMenu = CreateWidget<UUserWidget>(PlayerController, MainMenuClass);
    }

    if (DeathWidgetClass)
    {
        DeathWidget = CreateWidget<UUserWidget>(PlayerController, DeathWidgetClass);
    }

    if (FloorCompletedWidgetClass)
    {
        FloorCompletedWidget = CreateWidget<UUserWidget>(PlayerController, FloorCompletedWidgetClass);
    }

    if (TimeWidgetClass)
    {
        TimeWidget = CreateWidget<UUserWidget>(PlayerController, TimeWidgetClass);
    }
}

void APlayerBase::ShowWidgetToFocus(UUserWidget* WidgetToShow)
{
    WidgetToShow->AddToPlayerScreen();
    PlayerController->SetInputMode(FInputModeUIOnly());
    PlayerController->bShowMouseCursor = true;
}

void APlayerBase::HideFocusedWidget(UUserWidget* WidgetToHide)
{
    WidgetToHide->RemoveFromParent();
    PlayerController->SetInputMode(FInputModeGameOnly());
    PlayerController->bShowMouseCursor = false;
}

void APlayerBase::ShowWidgetAndPause(UUserWidget* WidgetToShow)
{
    ShowWidgetToFocus(WidgetToShow);
    UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void APlayerBase::HideWidgetAndUnpause(UUserWidget* WidgetToHide)
{
    HideFocusedWidget(WidgetToHide);
    UGameplayStatics::SetGamePaused(GetWorld(), false);
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
    float LadderCooldown = 0.2f;
    GetWorldTimerManager().SetTimer(LadderCooldownHandle, [this]() { bCanEnterLadder = true; }, LadderCooldown, false);
}

void APlayerBase::ExitLadderBoost()
{
    int LaunchForce = 50;
    LaunchCharacter(GetActorUpVector() * LaunchForce, false, false);
}

void APlayerBase::PauseUnpause()
{
    if (PauseWidget)
    {
        if(!PauseWidget->IsVisible())
        {
            ShowWidgetAndPause(PauseWidget);
        }
        else
        {
            HideWidgetAndUnpause(PauseWidget);
        }
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
        int LaunchFromLadderForce = 1000;
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
        LaunchCharacter(FVector(0,0,MovementComponent->JumpZVelocity), false, true);
    }
    else
    {
        Jump();
    }
}

void APlayerBase::Dash() // Dash ability
{
    if (!bDashOnCooldown && !bIsGrappling)
    {
        bDashOnCooldown = true;
        int DashForce;
        if (MovementComponent->IsFalling())
        {
            DashForce = 2550; // Dash is stronger if player is not in air
        }
        else
        {
            DashForce = 2700;
        }
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

void APlayerBase::CrouchSlide() // if player is fast enough we initiate slide if not we initiate normal crouch
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

void APlayerBase::BeginWallRun() // setting the values for wall running or movement and tilting camera
{
    MovementComponent->AirControl = 1;
    MovementComponent->GravityScale = 0;
    MovementComponent->SetPlaneConstraintNormal(FVector(0, 0, 1));
    bIsWallRunning = true;
    CameraTiltTimeline->Play();
    GetWorldTimerManager().SetTimer(WallRunTimer, this, &APlayerBase::UpdateWallRun, 0.1f, true);
}

void APlayerBase::EndWallRun() // setting the default values or movement and tilting camera back
{
    MovementComponent->AirControl = DefaultAirControl;
    MovementComponent->GravityScale = DefaultGravityScale;
    MovementComponent->SetPlaneConstraintNormal(FVector(0, 0, 0));
    bIsWallRunning = false;
    CameraTiltTimeline->Reverse();
    GetWorldTimerManager().ClearTimer(WallRunTimer);
}

void APlayerBase::UpdateWallRun() // wall run function called every tick when wallrunning setting velocity or ending the wall run
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
    FVector TraceEnd = GetActorLocation() + FVector::CrossProduct(WallRunDirection, SideToTrace) * TraceLength;
    bool bHitWall = GetWorld()->LineTraceSingleByChannel(HitResult, GetActorLocation(), TraceEnd, ECC_Visibility, FCollisionQueryParams(FName(NAME_None), false, this));
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

void APlayerBase::CameraTilt(float TimelineVal) // when wallrunning camera tilts a little bit
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
    float LaunchForceMultiplier = 0.55f;
    FVector AdditionalUpForce = FVector(0, 0, 150);
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
    PlayerController->SetIgnoreMoveInput(true);
    GrappleBeginningDistance = UKismetMathLibrary::Vector_Distance(GetActorLocation(), GrappleTarget->GetActorLocation());
    GrappleDragTimeline->PlayFromStart();
}

void APlayerBase::GrappleSecond()
{
    GrappleLine->GrappleOff();
    bIsGrappling = false;
    bGrappleCanSecondUse = false;
    PlayerController->SetIgnoreMoveInput(false);
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
    FVector GrappleForce = (MovementComponent->Velocity.GetSafeNormal() + ZImpulseToAddOnEnd) * GrappleImpulseMultiplier;
    MovementComponent->AddImpulse(GrappleForce);
}

void APlayerBase::GrappleDragUpdate(float TimelineVal)
{
    FVector GrappleDirection;
    GrappleDirection = (GrappleTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector GrappleForce;
    GrappleForce = GrappleDirection * GrappleForceMultiplier * MovementComponent->Mass * TimelineVal;
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
    float TraceRadius = GrappleRange;
    TArray<FHitResult> TraceResults;
    TArray<TEnumAsByte<EObjectTypeQuery>>GrappleTargetType;
    const EObjectTypeQuery GrappleType = UCollisionProfile::Get()->ConvertToObjectType(ECC_GameTraceChannel1);  // grapple object type
    if(GrappleType)GrappleTargetType.Add(GrappleType);
    UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), GetActorLocation(), GetActorLocation(), TraceRadius, GrappleTargetType, false, TArray<AActor*>(), EDrawDebugTrace::None, TraceResults, false);
    float Difference = 999999; // Difference in distance between grapple target and player's cursor
    FVector PlayerLooks = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetActorForwardVector();
    for( const auto& Hit : TraceResults)
    {
        AActor* TargetFound = Hit.GetActor();
        bool bTargetOnScreen = TargetFound->WasRecentlyRendered();
        FVector TargetDirection = FVector(TargetFound->GetActorLocation()-GetActorLocation()).GetSafeNormal();
        float DistanceTargetToCursor = UKismetMathLibrary::Vector_Distance(TargetDirection, PlayerLooks);
        bool bTargetCloserToCursor = DistanceTargetToCursor < Difference;
        if (bTargetOnScreen && bTargetCloserToCursor)
        {
            Difference = DistanceTargetToCursor;
            GrappleTarget = TargetFound;
        }
    }
    return UKismetSystemLibrary::IsValid(GrappleTarget);
}
