// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HeistFPSGameInstance.h"
#include "Engine/World.h"

void UHeistFPSGameInstance::LoadMap(FText MapURL) {
	GetWorld()->ServerTravel(MapURL.ToString());
}

