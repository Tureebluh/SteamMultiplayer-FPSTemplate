// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SessionBtn.h"

#include "Components/TextBlock.h"


void USessionBtn::SetSessionName(FText SessionNameText)
{
	SessionName->SetText(SessionNameText);
}