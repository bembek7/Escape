// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllerBase.h"
#include "Kismet/GameplayStatics.h"
#include "RegularStart.h"
#include "GMBase.h"
#include "GameFramework/PlayerStart.h"
#include "SaveGameBase.h"

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	TutorialStartLocation = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass())->GetActorLocation();
	RegularStartLocation = UGameplayStatics::GetActorOfClass(GetWorld(), ARegularStart::StaticClass())->GetActorLocation();
    // Deciding Spawn Location and teleporting player there
	DecideSpawnLocation();
    TeleportToSpawn();
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
    USaveGameBase* SaveObject;
    if (!UGameplayStatics::DoesSaveGameExist(USaveGameBase::SaveSlotName, USaveGameBase::SaveIndex))
    {
        SaveObject = Cast<USaveGameBase>(UGameplayStatics::CreateSaveGameObject(USaveGameBase::StaticClass()));
        UGameplayStatics::SaveGameToSlot(SaveObject, USaveGameBase::SaveSlotName, USaveGameBase::SaveIndex);
    }
    else
    {
        SaveObject = Cast<USaveGameBase>(UGameplayStatics::LoadGameFromSlot(USaveGameBase::SaveSlotName, USaveGameBase::SaveIndex));
    }
	bool bPlayedTutorial = SaveObject->bPlayedTutorial;
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

void APlayerControllerBase::SetSpawnLocationToRegular()
{
    PlayerSpawnLocation = RegularStartLocation;
}

void APlayerControllerBase::TeleportToSpawn() const
{
    UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->SetActorLocation(PlayerSpawnLocation);
}

FVector APlayerControllerBase::GetPlayerSpawnLocation() const
{
    return PlayerSpawnLocation;
}

void APlayerControllerBase::HideMainMenu()
{
    HideFocusedWidget(MainMenu);
}

void APlayerControllerBase::HideDeathWidget()
{
    HideWidgetAndUnpause(DeathWidget);
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

void APlayerControllerBase::ShowDeathWidget()
{
    ShowWidgetAndPause(DeathWidget);
}

void APlayerControllerBase::HideFloorCompletedWidget()
{
    HideWidgetAndUnpause(FloorCompletedWidget);
}
