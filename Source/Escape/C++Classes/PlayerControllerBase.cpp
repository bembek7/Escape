// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllerBase.h"
#include "Kismet/GameplayStatics.h"
#include "RegularStart.h"
#include "GMBase.h"
#include "GameFramework/PlayerStart.h"

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	TutorialStartLocation = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass())->GetActorLocation();
	RegularStartLocation = UGameplayStatics::GetActorOfClass(GetWorld(), ARegularStart::StaticClass())->GetActorLocation();
	DecideSpawnLocation();
	UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->SetActorLocation(PlayerSpawnLocation);
    CreateWidgets();
    HudWidget->AddToPlayerScreen();
    ShowWidgetToFocus(MainMenu);
}

void APlayerControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerControllerBase::DecideSpawnLocation()
{
	bool bPlayedTutorial = Cast<AGMBase>(UGameplayStatics::GetGameMode(GetWorld()))->PlayedTutorial();
	if (bPlayedTutorial)
	{
		PlayerSpawnLocation = RegularStartLocation;
		
	}
	else
	{
        PlayerSpawnLocation = TutorialStartLocation;
        bInTutorial = true;
	}
}


void APlayerControllerBase::CreateWidgets()
{
    {
        HudWidget = CreateWidget<UW_HUD>(this, HudWidgetClass);
    }

    if (PauseWidgetClass)
    {
        PauseWidget = CreateWidget<UUserWidget>(this, PauseWidgetClass);
    }

    if (MainMenuClass)
    {
        MainMenu = CreateWidget<UUserWidget>(this, MainMenuClass);
    }

    if (DeathWidgetClass)
    {
        DeathWidget = CreateWidget<UUserWidget>(this, DeathWidgetClass);
    }

    if (FloorCompletedWidgetClass)
    {
        FloorCompletedWidget = CreateWidget<UUserWidget>(this, FloorCompletedWidgetClass);
    }

    if (TimeWidgetClass)
    {
        TimeWidget = CreateWidget<UUserWidget>(this, TimeWidgetClass);
    }
}

void APlayerControllerBase::ShowWidgetToFocus(UUserWidget* WidgetToShow)
{
    WidgetToShow->AddToPlayerScreen();
    SetInputMode(FInputModeUIOnly());
    bShowMouseCursor = true;
}

void APlayerControllerBase::HideFocusedWidget(UUserWidget* WidgetToHide)
{
    WidgetToHide->RemoveFromParent();
    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;
}

void APlayerControllerBase::ShowWidgetAndPause(UUserWidget* WidgetToShow)
{
    ShowWidgetToFocus(WidgetToShow);
    UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void APlayerControllerBase::HideWidgetAndUnpause(UUserWidget* WidgetToHide)
{
    HideFocusedWidget(WidgetToHide);
    UGameplayStatics::SetGamePaused(GetWorld(), false);
}

void APlayerControllerBase::PauseUnpause()
{
    if (PauseWidget)
    {
        if (!PauseWidget->IsVisible())
        {
            ShowWidgetAndPause(PauseWidget);
        }
        else
        {
            HideWidgetAndUnpause(PauseWidget);
        }
    }
}

void APlayerControllerBase::UpdateDashIconScanHudWidget(float Percent)
{
    HudWidget->UpdateDashIconScan(Percent);
}

void APlayerControllerBase::TeleportToTutorial()
{
    UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->SetActorLocation(TutorialStartLocation);
    PlayerSpawnLocation = TutorialStartLocation;
    bInTutorial = true;
}

void APlayerControllerBase::HideMainMenu()
{
    HideFocusedWidget(MainMenu);
}

void APlayerControllerBase::ShowTimeWidget()
{
    TimeWidget->AddToPlayerScreen();
}

void APlayerControllerBase::HideTimeWidget()
{
    TimeWidget->RemoveFromParent();
}

void APlayerControllerBase::ShowFloorCompletedWidget()
{
    ShowWidgetAndPause(FloorCompletedWidget);
}

void APlayerControllerBase::HideFloorCompletedWidget()
{
    HideWidgetAndUnpause(FloorCompletedWidget);
}
