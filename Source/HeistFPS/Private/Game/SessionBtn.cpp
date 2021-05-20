// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SessionBtn.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "Game/MainMenu.h"


void USessionBtn::SetSessionName(FText SessionNameText)
{
	if (!ensure(SessionName != nullptr)) { return; }
	SessionName->SetText(SessionNameText);
}

void USessionBtn::Setup(class UMainMenu* InParent, uint32 InIndex)
{
	if (!ensure(InParent != nullptr && InIndex >= 0)) { return; }
	Parent = InParent;
	Index = InIndex;

	JoinSessionBtn->OnClicked.AddDynamic(this, &USessionBtn::OnClicked);
}

void USessionBtn::OnClicked()
{
	if (!ensure(Parent != nullptr)) { return; }
	Parent->SetSelectedSession(Index);
}