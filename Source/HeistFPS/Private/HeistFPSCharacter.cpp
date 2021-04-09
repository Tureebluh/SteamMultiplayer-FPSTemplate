// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeistFPSCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimInstance.h"

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

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void AHeistFPSCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHeistFPSCharacter, CurrentSpeed);
	DOREPLIFETIME(AHeistFPSCharacter, CurrentDirection);
	DOREPLIFETIME(AHeistFPSCharacter, CurrentPitch);
	DOREPLIFETIME(AHeistFPSCharacter, CurrentYaw);
	DOREPLIFETIME(AHeistFPSCharacter, bCombatInitiated);
	DOREPLIFETIME(AHeistFPSCharacter, bPrimaryEquipped);
	DOREPLIFETIME(AHeistFPSCharacter, bAimDownSight);
	DOREPLIFETIME(AHeistFPSCharacter, bIsSprinting);
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
	//GEngine->AddOnScreenDebugMessage(1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Normalized Direction - %d"), CurrentDirection));
	UpdateCharacterAnimMovement(DeltaTime);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AHeistFPSCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AHeistFPSCharacter::ToggleCrouch);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AHeistFPSCharacter::ToggleSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AHeistFPSCharacter::ToggleSprint);

	PlayerInputComponent->BindAction("EquipPrimaryWeapon", IE_Pressed, this, &AHeistFPSCharacter::EquipPrimaryWeapon);

	PlayerInputComponent->BindAction("AimDownSight", IE_Pressed, this, &AHeistFPSCharacter::AimDownSight);
	PlayerInputComponent->BindAction("AimDownSight", IE_Released, this, &AHeistFPSCharacter::AimDownSight);

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

void AHeistFPSCharacter::ToggleSprint()
{
	ServerToggleSprint();
}

void AHeistFPSCharacter::OnRep_bIsSprinting()
{
	if (!HasAuthority()) {
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
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AHeistFPSCharacter::UpdateCharacterAnimMovement(float DeltaTime)
{
	if (Controller != nullptr) {
		//Get difference between rotation of view and character yaw
		FRotator ActorRotation = GetActorRotation();
		FRotator ControllerRotation = Controller->GetControlRotation();
		FRotator DeltaRotation = FRotator(0.0f, ControllerRotation.Yaw - ActorRotation.Yaw, 0.0f);

		//Turn off rotation if rotation difference is less than 45 degrees
		if (FMath::IsNearlyEqual(DeltaRotation.Yaw, 0.0f, 20.0f)) {
			bAimOffsetRotation = false;
		}
		//If player is turning 90 degrees or more - interp rotate character
		if (!bCombatInitiated) {
			AutoRotationThreshold = 20.0f;
		}
		else {
			AutoRotationThreshold = 60.0f;
		}
		if ((FMath::Abs(DeltaRotation.Yaw) >= AutoRotationThreshold) || bAimOffsetRotation) {
			bAimOffsetRotation = true;
			FRotator InterpRotation = FMath::RInterpTo(FRotator(0.0f, 0.0f, 0.0f), DeltaRotation, DeltaTime, 2.5f);
			AddActorWorldRotation(InterpRotation);
		}

		//
		//CALCULATE MOVEMENT DIRECTION FOR ANIMATION
		//

		//Calculate speed
		CurrentSpeed = GetVelocity().Size();
		//Get normalized forward and velocity vectors
		FVector NormalizedForward = GetActorForwardVector().GetSafeNormal();
		FVector NormalizedVelocity = GetVelocity().GetSafeNormal();
		//Get inverse-cos of the dot product of the normalized vectors
		CurrentDirection = FMath::RadiansToDegrees(acosf(FVector::DotProduct(NormalizedVelocity, NormalizedForward)));

		//
		//CALCULATE AIM OFFSET FOR ANIMATION
		//
		if (bCombatInitiated) {
			//Get difference between view rotation and character rotation
			FRotator FullDeltaRotation = ControllerRotation - ActorRotation;
			FullDeltaRotation.Yaw = FMath::ClampAngle(FullDeltaRotation.Yaw, -90.0f, 90.0f);
			FullDeltaRotation.Pitch = FMath::ClampAngle(FullDeltaRotation.Pitch, -90.0f, 90.0f);
			//Zero out roll rotation
			FullDeltaRotation.Roll = 0.0f;
			
			//Interp between current rotation and difference
			FRotator InterpRotation = FMath::RInterpTo(FRotator(CurrentPitch, CurrentYaw, 0.0f), FullDeltaRotation, DeltaTime, 8.0f);
			//Set next pitch and yaw from interp
			CurrentPitch = InterpRotation.Pitch;
			CurrentYaw = InterpRotation.Yaw;
		}
	}
}

void AHeistFPSCharacter::EquipPrimaryWeapon()
{
	ServerEquipPrimaryWeapon();
}

void AHeistFPSCharacter::AimDownSight()
{
	ServerAimDownSight();
}

bool AHeistFPSCharacter::ServerAimDownSight_Validate() {
	return true;
}
void AHeistFPSCharacter::ServerAimDownSight_Implementation(){
	if (HasAuthority()) {
		bAimDownSight = !bAimDownSight;
	}
}

bool AHeistFPSCharacter::ServerEquipPrimaryWeapon_Validate() {
	return true;
}
void AHeistFPSCharacter::ServerEquipPrimaryWeapon_Implementation() {
	if (HasAuthority()) {
		if (bPrimaryEquipped) {
			bCombatInitiated = false;
			bPrimaryEquipped = false;
		}
		else {
			bPrimaryEquipped = true;
			bCombatInitiated = true;
		}
	}
}

bool AHeistFPSCharacter::ServerToggleSprint_Validate() {
	return true;
}
void AHeistFPSCharacter::ServerToggleSprint_Implementation() {
	if (HasAuthority()) {
		if (!bIsSprinting) {
			bIsSprinting = true;
			GetCharacterMovement()->MaxWalkSpeed = MaxSprintSpeed;
		}
		else {
			bIsSprinting = false;
			GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;
		}
	}
}