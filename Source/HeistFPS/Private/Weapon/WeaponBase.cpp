// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/WeaponBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Camera/CameraComponent.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	bReplicates = true;

	// Create a camera
	ADSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ADSCamera"));
	
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	if (WeaponMesh)
	{
		ADSCamera->AttachToComponent(WeaponMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ADSCameraAttachPoint);
		ADSCamera->Activate(true);
	}
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//Play cosmetic aspects of weapon firing - FX etc.
void AWeaponBase::SimulateWeaponFire()
{
	if (WeaponMesh && MuzzleFX) {
		FVector LocationOffset = FVector(0.0f, 0.0f, 0.0f);
		FRotator RotationOffset = FRotator(90.0f, 0.0f, 0.0f);
		MuzzlePSC = UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFX, WeaponMesh, MuzzleAttachPoint, LocationOffset, RotationOffset, EAttachLocation::KeepRelativeOffset, true);
	}
}

