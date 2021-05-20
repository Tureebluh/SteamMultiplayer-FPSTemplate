// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HeistFPSGameInstance.h"

#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

#include "Game/MainMenu.h"

const static FName SESSION_NAME = TEXT("My Session");

UHeistFPSGameInstance::UHeistFPSGameInstance(const FObjectInitializer &ObjectInitializer)
{
	//Return if PauseMenu is not found
	ConstructorHelpers::FClassFinder<UUserWidget>PauseMenuBPClass(TEXT("/Game/UI/WBP_PauseMenu"));
	if (!ensure(PauseMenuBPClass.Class != nullptr)) { return; }
	PauseMenuClass = PauseMenuBPClass.Class;

	//Return if MainMenu is not found
	ConstructorHelpers::FClassFinder<UUserWidget>MainMenuBPClass(TEXT("/Game/UI/WBP_MainMenu"));
	if (!ensure(MainMenuBPClass.Class != nullptr)) { return; }
	MainMenuClass = MainMenuBPClass.Class;
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
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UHeistFPSGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UHeistFPSGameInstance::OnJoinSessionComplete);
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Subsystem not found."));
	}
}

void UHeistFPSGameInstance::LoadMainMenu()
{
	//Return if MainMenuClass is null
	if (!ensure(MainMenuClass != nullptr)) { UE_LOG(LogTemp, Warning, TEXT("MainMenuClass not found.")); return; }
	
	MainMenu = CreateWidget<UMainMenu>(this, MainMenuClass);
	MainMenu->SetMenuInterface(this);
	MainMenu->Setup();
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

void UHeistFPSGameInstance::HostMap()
{
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

void UHeistFPSGameInstance::RefreshServerList()
{
	if (!SessionInterface.IsValid()) { return; }
	if (!ensure(MainMenu != nullptr)) { return; }
	MainMenu->ClearServerList();

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid())
	{
		SessionSearch->bIsLanQuery = true;
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UHeistFPSGameInstance::CreateSession()
{
	//Check if SessionInterface is valid and print error to console if not
	if (!SessionInterface.IsValid()) { UE_LOG(LogTemp, Warning, TEXT("SessionInterface is not valid")); return; }

	//Create session with specified session name
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.NumPublicConnections = 4;
	SessionInterface->CreateSession(0, FName(SESSION_NAME), SessionSettings);
}

void UHeistFPSGameInstance::OnCreateSessionComplete(FName SessionName, bool Success)
{
	//Return and print error to console if session creation fails
	if (!Success) { UE_LOG(LogTemp, Warning, TEXT("Failed to create session.")); return; }

	//Return if world does not exist
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) { return; }

	if (MainMenu != nullptr)
	{
		MainMenu->Teardown();
	}

	//Load map as listening server
	World->ServerTravel("/Game/Maps/Test/Test1?listen");
}

void UHeistFPSGameInstance::OnDestroySessionComplete(FName SessionName, bool Success)
{
	//If destroy session fails - print to console and early return
	if (!Success) { UE_LOG(LogTemp, Warning, TEXT("Failed to destroy session.")); return; }
	
	CreateSession();
}

void UHeistFPSGameInstance::JoinMap(uint32 SessionIndex)
{
	if (!SessionInterface.IsValid()) { return; }
	if (!SessionSearch.IsValid()) { return; }
	if (MainMenu != nullptr)
	{
		MainMenu->Teardown();
	}

	SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[SessionIndex]);
}

void UHeistFPSGameInstance::OnFindSessionsComplete(bool Success)
{
	if (!Success) { UE_LOG(LogTemp, Warning, TEXT("Failed to find sessions.")); return; }
	if (!SessionSearch.IsValid()) { UE_LOG(LogTemp, Warning, TEXT("SessionSearch is not valid.")); return; }

	TArray<FString> SessionNames;

	for (FOnlineSessionSearchResult& Session : SessionSearch->SearchResults)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found session: %s"), *Session.GetSessionIdStr());
		SessionNames.Add(Session.GetSessionIdStr());
	}
	if (!ensure(MainMenu != nullptr)) { return; }
	MainMenu->SetServerList(SessionNames);
}

void UHeistFPSGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid()) { return; }

	FString IpAddress;
	if (!SessionInterface->GetResolvedConnectString(SessionName, IpAddress)) { UE_LOG(LogTemp, Warning, TEXT("Could not resolve connection string.")); return; }

	//Return if PlayerController is null
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) { return; }
	
	//Load map at specified IP address as client
	PlayerController->ClientTravel(IpAddress, ETravelType::TRAVEL_Absolute);
}

void UHeistFPSGameInstance::QuitGame()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) { return; }

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!ensure(PlayerController != nullptr)) { return; }

	PlayerController->ConsoleCommand("quit");
}