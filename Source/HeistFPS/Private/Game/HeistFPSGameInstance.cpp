// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HeistFPSGameInstance.h"

#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

const static FName SESSION_NAME = TEXT("My Session");

UHeistFPSGameInstance::UHeistFPSGameInstance(const FObjectInitializer &ObjectInitializer)
{
	//Return if PauseMenu is not found
	ConstructorHelpers::FClassFinder<UUserWidget>PauseMenuBPClass(TEXT("/Game/UI/WBP_PauseMenu"));
	if (!ensure(PauseMenuBPClass.Class != nullptr)) { return; }

	PauseMenuClass = PauseMenuBPClass.Class;
}

void UHeistFPSGameInstance::Init()
{
	//Get Online SubSystem
	IOnlineSubsystem* SubSystem = IOnlineSubsystem::Get();

	//Log error to console if SubSystem is null
	if (SubSystem != nullptr)
	{
		//Check if interface is valid and register OnCreate and OnDestroy Session Events
		SessionInterface = SubSystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UHeistFPSGameInstance::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UHeistFPSGameInstance::OnDestroySessionComplete);
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Subsystem not found."));
	}
}

void UHeistFPSGameInstance::TogglePauseMenu()
{
	//Return if PauseMenuClass is null
	if (!ensure(PauseMenuClass != nullptr)) { return; }
	//Only Create PauseMenu widget if it hasn't been created previously
	if(PauseMenu == nullptr)
	{
		PauseMenu = CreateWidget<UUserWidget>(this, PauseMenuClass);
	}
	
	//Return if PauseMenu or PlayerController are null
	if (!ensure(PauseMenu != nullptr && GetFirstLocalPlayerController() != nullptr)) { return; }
	if (PauseMenu->IsInViewport())
	{
		//Remove pause menu from viewport and return input mode back to game only
		PauseMenu->RemoveFromViewport();
		FInputModeGameOnly InputModeData;
		GetFirstLocalPlayerController()->SetInputMode(InputModeData);
		GetFirstLocalPlayerController()->bShowMouseCursor = false;
	}
	else
	{
		//Add pause menu to viewport with specified z-index
		PauseMenu->AddToViewport(1);
		//Set GameAndUI input mode, enable cursor and set focus to pause menu
		FInputModeGameAndUI InputModeData;
		InputModeData.SetWidgetToFocus(PauseMenu->TakeWidget());
		GetFirstLocalPlayerController()->SetInputMode(InputModeData);
		GetFirstLocalPlayerController()->bShowMouseCursor = true;
	}
}

void UHeistFPSGameInstance::HostMap(FText MapURL)
{
	//Initialize map to be loaded
	SelectedHostMap = MapURL;

	//Check if session already exist and destroy if true - otherwise create session
	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
	if (ExistingSession != nullptr)
	{
		SessionInterface->DestroySession(SESSION_NAME);
	}
	else {
		CreateSession();
	}
}

void UHeistFPSGameInstance::OnCreateSessionComplete(FName SessionName, bool Success)
{
	//Return and print error to console if session creation fails
	if (!Success) { UE_LOG(LogTemp, Warning, TEXT("Failed to create session.")); return; }
	//Return if selected host map name is empty
	if (SelectedHostMap.EqualTo(FText::GetEmpty(), ETextComparisonLevel::Default)) { return; }

	//Return if world does not exist
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) { return; }

	//Load map as listening server
	World->ServerTravel(SelectedHostMap.ToString() + "?listen");
}

void UHeistFPSGameInstance::OnDestroySessionComplete(FName SessionName, bool Success)
{
	//If destroy session fails - print to console and early return
	if (!Success) { UE_LOG(LogTemp, Warning, TEXT("Failed to destroy session.")); return; }
	
	CreateSession();
}

void UHeistFPSGameInstance::CreateSession()
{
	//Check if SessionInterface is valid and print error to console if not
	if (!SessionInterface.IsValid()) { UE_LOG(LogTemp, Warning, TEXT("SessionInterface is not valid")); return; }

	//Create session with specified session name
	FOnlineSessionSettings SessionSettings;
	SessionInterface->CreateSession(0, FName(SESSION_NAME), SessionSettings);
}

void UHeistFPSGameInstance::JoinMap(const FString& IpAddress)
{
	//Return if PlayerController is null
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) { return; }
	
	//Load map at specified IP address as client
	PlayerController->ClientTravel(IpAddress, ETravelType::TRAVEL_Absolute);
}