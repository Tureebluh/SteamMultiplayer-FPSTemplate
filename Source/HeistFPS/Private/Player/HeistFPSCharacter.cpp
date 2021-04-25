// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HeistFPSCharacter.h"

#include "Weapon/WeaponBase.h"
#include "Game/HeistFPSGameInstance.h"

#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"

//////////////////////////////////////////////////////////////////////////
// AHeistFPSCharacter

AHeistFPSCharacter::AHeistFPSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false;	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = MaxCrouchSpeed;

	// Create a camera
	FPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPSCamera"));

}

/********************************************************************
				REPLICATE VARIABLES
*********************************************************************/
void AHeistFPSCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AHeistFPSCharacter, CurrentSpeed, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHeistFPSCharacter, CurrentDirection, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHeistFPSCharacter, CurrentPitch, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHeistFPSCharacter, CurrentYaw, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHeistFPSCharacter, LastMoveRightValue, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHeistFPSCharacter, bCombatInitiated, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHeistFPSCharacter, bPrimaryEquipped, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHeistFPSCharacter, bAimDownSight, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AHeistFPSCharacter, Inventory, COND_OwnerOnly);
}

void AHeistFPSCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		// Needs to happen after character is added to repgraph
		GetWorldTimerManager().SetTimerForNextTick(this, &AHeistFPSCharacter::SpawnDefaultInventory);
	}
}
void AHeistFPSCharacter::BeginPlay() {
	Super::BeginPlay();
	if (GetMesh()) {
		FPSCamera->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("head"));
		FPSCamera->AddLocalOffset(FVector(0.0f, 8.0f, 0.0f));
		FPSCamera->bUsePawnControlRotation = true;
	}
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	bUseControllerRotationYaw = false;
}
void AHeistFPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//GEngine->AddOnScreenDebugMessage(1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Normalized Direction - %f"), CurrentDirection));
	//UE_LOG(LogTemp, Warning, TEXT("Some warning message"));
	//FString NetMode = GEngine->GetNetMode(GetWorld()) == NM_Client ? TEXT("Client") : TEXT("Server");
	//UE_LOG(LogTemp, Warning, TEXT("%s is running StartSprint()"), *NetMode);
	UpdateCharacterAnimMovement(DeltaTime);
}

/********************************************************************
				UPDATE CHARACTER ANIMATION VARIABLES
*********************************************************************/
void AHeistFPSCharacter::UpdateCharacterAnimMovement(float DeltaTime)
{
	if (Controller != nullptr) {
		/********************************************************************
					GET DELTA ROTATION FOR AUTO-ROTATION
		*********************************************************************/
		FRotator ActorRotation = GetActorRotation();
		FRotator ControllerRotation = Controller->GetControlRotation();
		FRotator DeltaRotation = FRotator(0.0f, ControllerRotation.Yaw - ActorRotation.Yaw, 0.0f);

		//Turn off rotation if rotation difference is less than n degrees
		if (FMath::IsNearlyEqual(DeltaRotation.Yaw, 0.0f, 20.0f)) {
			bAimOffsetRotation = false;
		}
		//If player is turning past the autorotationthreshold  - interp rotate character
		//If player has no weapon, turn more frequently
		if (!bCombatInitiated) {
			AutoRotationThreshold = 20.0f;
		}
		else {
			AutoRotationThreshold = 60.0f;
		}
		if ((FMath::Abs(DeltaRotation.Yaw) >= AutoRotationThreshold) || bAimOffsetRotation) {
			bAimOffsetRotation = true;
			FRotator InterpRotation = FMath::RInterpTo(FRotator(0.0f, 0.0f, 0.0f), DeltaRotation, DeltaTime, 2.0f);
			AddActorWorldRotation(InterpRotation);
		}

		/********************************************************************
					CALCULATE MOVEMENT DIRECTION FOR ANIMATIONS
		*********************************************************************/

		//Calculate speed
		CurrentSpeed = GetVelocity().Size();
		//Get normalized forward and velocity vectors
		FVector NormalizedForward = GetActorForwardVector().GetSafeNormal();
		FVector NormalizedVelocity = GetVelocity().GetSafeNormal();
		//Get inverse-cos of the dot product of the normalized vectors to return degree between two vectors
		//Multiple that result by the last input to also get the correct direction (i.e angle will always be between 0-180)
		float DotProduct = FVector::DotProduct(NormalizedVelocity, NormalizedForward);
		float AngleInRadians = acosf(FMath::Clamp(DotProduct, -1.0f, 1.0f));
		float AngleInDegrees = FMath::RadiansToDegrees(AngleInRadians);
		CurrentDirection = AngleInDegrees * LastMoveRightValue;


		/********************************************************************
								CALCULATE AIM OFFSET
		*********************************************************************/
		if (bCombatInitiated) {
			//Get difference between view rotation and character rotation
			FRotator FullDeltaRotation = ControllerRotation - ActorRotation;
			FullDeltaRotation.Yaw = FMath::ClampAngle(FullDeltaRotation.Yaw, -90.0f, 90.0f);
			FullDeltaRotation.Pitch = FMath::ClampAngle(FullDeltaRotation.Pitch, -90.0f, 90.0f);
			//Zero out roll rotation
			FullDeltaRotation.Roll = 0.0f;

			//Interp between current rotation and difference
			FRotator InterpRotation = FMath::RInterpTo(FRotator(CurrentPitch, CurrentYaw, 0.0f), FullDeltaRotation, DeltaTime, 15.0f);
			//Set next pitch and yaw from interp
			CurrentPitch = InterpRotation.Pitch;
			CurrentYaw = InterpRotation.Yaw;
		}
	}
}

void AHeistFPSCharacter::SpawnDefaultInventory()
{
	if (!HasAuthority())
	{
		return;
	}

	for (int32 i = 0; i < DefaultWeaponClasses.Num(); i++)
	{
		if (DefaultWeaponClasses[i])
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(DefaultWeaponClasses[i], SpawnInfo);
			AddWeapon(NewWeapon);
		}
	}
	if (Inventory.Num() > 0)
	{
		Inventory[0]->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("RifleEquipSocket"));
		Inventory[0]->SetActorHiddenInGame(true);
	}
}

void AHeistFPSCharacter::AddWeapon(AWeaponBase* Weapon) {
	if (Weapon && HasAuthority())
	{
		Inventory.AddUnique(Weapon);
	}
}

/********************************************************************
				SETUP INPUT EVENTS
*********************************************************************/

void AHeistFPSCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AHeistFPSCharacter::ToggleCrouch);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AHeistFPSCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AHeistFPSCharacter::StopSprint);

	PlayerInputComponent->BindAction("TogglePrimaryWeapon", IE_Pressed, this, &AHeistFPSCharacter::TogglePrimaryWeapon);
	PlayerInputComponent->BindAction("FireWeapon", IE_Pressed, this, &AHeistFPSCharacter::FireWeapon);

	PlayerInputComponent->BindAction("AimDownSight", IE_Pressed, this, &AHeistFPSCharacter::AimDownSight);
	PlayerInputComponent->BindAction("AimDownSight", IE_Released, this, &AHeistFPSCharacter::AimDownSight);

	PlayerInputComponent->BindAction("TogglePauseMenu", IE_Pressed, this, &AHeistFPSCharacter::TogglePauseMenu);

	PlayerInputComponent->BindAxis("MoveForward", this, &AHeistFPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHeistFPSCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AHeistFPSCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AHeistFPSCharacter::LookUpAtRate);

}

/********************************************************************
				TOGGLE CROUCH CLIENT & SERVER
*********************************************************************/
void AHeistFPSCharacter::TogglePauseMenu()
{
	UHeistFPSGameInstance* GI = Cast<UHeistFPSGameInstance>(GetGameInstance());
	if (!ensure(GI != nullptr)) { return; }
	GI->TogglePauseMenu();
}

/********************************************************************
				TOGGLE CROUCH CLIENT & SERVER
*********************************************************************/
void AHeistFPSCharacter::ToggleCrouch()
{
	if (!bCombatInitiated) {
		return;
	}
	if (GetCharacterMovement()->IsCrouching()) {
		UnCrouch();
	}
	else {
		Crouch();
	}
}

/********************************************************************
				TOGGLE SPRINT CLIENT & SERVER
*********************************************************************/
void AHeistFPSCharacter::StartSprint()
{
	if (!HasAuthority()) {
		GetCharacterMovement()->MaxWalkSpeed = MaxSprintSpeed;
		ServerToggleSprint(true);
	}
	else {
		ServerToggleSprint(true);
	}
}
void AHeistFPSCharacter::StopSprint() 
{
	if (!HasAuthority()) {
		GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;
		ServerToggleSprint(false);
	}
	else {
		ServerToggleSprint(false);
	}
}
bool AHeistFPSCharacter::ServerToggleSprint_Validate(bool bIsSprinting) {
	return true;
}
void AHeistFPSCharacter::ServerToggleSprint_Implementation(bool bIsSprinting) {
	if (HasAuthority()) {
		if (bIsSprinting) {
			GetCharacterMovement()->MaxWalkSpeed = MaxSprintSpeed;
		}
		else {
			GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;
		}
	}
}

void AHeistFPSCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AHeistFPSCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AHeistFPSCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AHeistFPSCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		LastMoveRightValue = Value;
		if (!HasAuthority()){
			ServerUpdateLastMoveRight(Value);
		}
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
bool AHeistFPSCharacter::ServerUpdateLastMoveRight_Validate(float Value) {
	return true;
}
void AHeistFPSCharacter::ServerUpdateLastMoveRight_Implementation(float Value) {
	if (HasAuthority()) {
		LastMoveRightValue = Value;
	}
}

void AHeistFPSCharacter::FireWeapon() {
	if (bCombatInitiated) {
		Inventory[0]->SimulateWeaponFire();
	}
}

/********************************************************************
				TOGGLE PRIMARY WEAPON CLIENT & SERVER
*********************************************************************/
void AHeistFPSCharacter::TogglePrimaryWeapon()
{
	if (!HasAuthority()) {
		ServerTogglePrimaryWeapon(!bPrimaryEquipped);
	}
	if (!bPrimaryEquipped)
	{
		Inventory[0]->SetActorHiddenInGame(false);
	}
	else {
		Inventory[0]->SetActorHiddenInGame(true);
	}
	bCombatInitiated = !bCombatInitiated;
	bPrimaryEquipped = !bPrimaryEquipped;
}
bool AHeistFPSCharacter::ServerTogglePrimaryWeapon_Validate(bool IsEquipping) {
	return true;
}
void AHeistFPSCharacter::ServerTogglePrimaryWeapon_Implementation(bool IsEquipping) {
	if (HasAuthority()) {
		bCombatInitiated = IsEquipping;
		bPrimaryEquipped = IsEquipping;
		if (IsEquipping)
		{
			Inventory[0]->SetActorHiddenInGame(false);
		}
		else {
			Inventory[0]->SetActorHiddenInGame(true);
		}
	}
}

/********************************************************************
				AIMDOWNSIGHT CLIENT & SERVER
*********************************************************************/
void AHeistFPSCharacter::AimDownSight()
{
	if (!bCombatInitiated) { return; }

	if (!HasAuthority()) {
		ServerAimDownSight();
	}
	bAimDownSight = !bAimDownSight;

	if (bAimDownSight) {
		FPSCamera->Activate(false);
		Inventory[0]->GetADSCamera()->Activate(true);
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		PC->SetViewTargetWithBlend(Inventory[0], 0.25f);
	}
	else {
		FPSCamera->Activate(true);
		Inventory[0]->GetADSCamera()->Activate(false);
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		PC->SetViewTargetWithBlend(this, 0.25f);
	}
}
bool AHeistFPSCharacter::ServerAimDownSight_Validate() {
	return true;
}
void AHeistFPSCharacter::ServerAimDownSight_Implementation(){
	if (HasAuthority()) {
		bAimDownSight = !bAimDownSight;
	}
}