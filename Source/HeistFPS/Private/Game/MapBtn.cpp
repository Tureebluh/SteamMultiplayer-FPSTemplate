// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/MapBtn.h"

UMapBtn::UMapBtn(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	
}

bool UMapBtn::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) { UE_LOG(LogTemp, Warning, TEXT("MapBtn failed to initialize.")); return false; }
	return true;
}