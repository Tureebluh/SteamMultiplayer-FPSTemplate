// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HeistFPSGameInstance.h"

#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"

UHeistFPSGameInstance::UHeistFPSGameInstance(const FObjectInitializer &ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget>PauseMenuBPClass(TEXT("/Game/UI/WBP_PauseMenu"));
	if (!ensure(PauseMenuBPClass.Class != nullptr)) { return; }

	PauseMenuClass = PauseMenuBPClass.Class;
}

void UHeistFPSGameInstance::Init()
{
	Super::Init();
	
}

void UHeistFPSGameInstance::TogglePauseMenu()
{
	if (!ensure(PauseMenuClass != nullptr)) { return; }
	if(PauseMenu == nullptr)
	{
		PauseMenu = CreateWidget<UUserWidget>(this, PauseMenuClass);
	}
	
	if (!ensure(PauseMenu != nullptr && GetFirstLocalPlayerController() != nullptr)) { return; }
	if (PauseMenu->IsInViewport())
	{
		PauseMenu->RemoveFromViewport();
		FInputModeGameOnly InputModeData;
		GetFirstLocalPlayerController()->SetInputMode(InputModeData);
		GetFirstLocalPlayerController()->bShowMouseCursor = false;
	}
	else
	{
		PauseMenu->AddToViewport(1);
		FInputModeGameAndUI InputModeData;
		InputModeData.SetWidgetToFocus(PauseMenu->TakeWidget());
		GetFirstLocalPlayerController()->SetInputMode(InputModeData);
		GetFirstLocalPlayerController()->bShowMouseCursor = true;
	}
}

void UHeistFPSGameInstance::HostMap(FText MapURL)
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) { return; }

	World->ServerTravel(MapURL.ToString() + "?listen");
}

void UHeistFPSGameInstance::JoinMap(const FString& IpAddress)
{
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) { return; }
	
	PlayerController->ClientTravel(IpAddress, ETravelType::TRAVEL_Absolute);
}

