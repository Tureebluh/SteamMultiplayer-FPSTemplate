// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeistFPSGameMode.h"
#include "Player/HeistFPSCharacter.h"
#include "UObject/ConstructorHelpers.h"

AHeistFPSGameMode::AHeistFPSGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/HeistFPSCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
