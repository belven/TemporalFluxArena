// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "TemporalFluxArena.h"
#include "Ship.h"
#include "BaseProjectile.h"
#include "Weapon.h"
#include "SpeedBoost.h"
#include "TimerManager.h"
#include "Engine.h"
#include "ShipController.h"
#include "UnrealNetwork.h"
#include "ProjectileData.h"

const FName AShip::MoveForwardBinding("MoveForward");
const FName AShip::MoveRightBinding("MoveRight");
const FName AShip::FireForwardBinding("FireForward");
const FName AShip::FireRightBinding("FireRight");

AShip::AShip()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShipMesh(TEXT("/Game/TwinStick/Meshes/TwinStickUFO.TwinStickUFO"));
	// Create the mesh component
	ShipMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMeshComponent;
	ShipMeshComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	ShipMeshComponent->SetStaticMesh(ShipMesh.Object);

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->AttachTo(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Don't want arm to rotate when ship does
	CameraBoom->TargetArmLength = 1500.f;
	CameraBoom->RelativeRotation = FRotator(-80.f, 0.f, 0.f);
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	CameraComponent->AttachTo(CameraBoom, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	aoeDecetion = CreateDefaultSubobject<USphereComponent>(TEXT("AoeSphere"));
	aoeDecetion->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	aoeDecetion->SetSphereRadius(2000);

	bReplicates = true;
	bReplicateMovement = true;

	location = FVector(0, 0, 0);
	
	FProjectileData data;
	data.cost = 20;
	data.damage = 80;
	data.speed = 1.2;

	weapons = *new TArray<UWeapon*>();
	weapons.Empty();

	weapons.Add(UWeapon::CreateWeapon(this, 1, FVector(90.f, 0.f, 0.f), data));

	data.cost = 5;
	data.damage = 2;
	data.speed = 4;
	weapons.Add(UWeapon::CreateWeapon(this, 0.1, FVector(90.f, 0.f, 0.f), data));

	data.cost = 15;
	data.damage = 120;
	data.speed = 6;
	weapons.Add(UWeapon::CreateWeapon(this, 2, FVector(90.f, 0.f, 0.f), data));

	currentWeapon = weapons[0];

	abilities = *new TArray<UAbility*>();
	abilities.Add(USpeedBoost::CreateAbility());
	abilities[0]->SetCooldown(3);
}

void AShip::OnRep_LocationChanged()
{
	RootComponent->SetWorldLocation(location);
}

void AShip::OnRep_RotationChanged()
{
	RootComponent->SetWorldRotation(rotation);
}

void AShip::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	//DOREPLIFETIME(AShip, health);
	//DOREPLIFETIME(AShip, shield);
	//DOREPLIFETIME(AShip, energy);
	DOREPLIFETIME(AShip, location);
	DOREPLIFETIME(AShip, rotation);
}

void AShip::PossessedBy(class AController* NewController){
	if (NewController->GetClass()->IsChildOf(AShipController::StaticClass())){
		Cast<AShipController>(NewController)->owningShip = this;
		shipController = Cast<AShipController>(NewController);
	}
}

// Called when the game starts or when spawned
void AShip::BeginPlay()
{
	Super::BeginPlay();

	// Movement
	MoveSpeed = 1000.0f;

	//Starting ship data to retain original stats
	originalData.maxHealth = 100.0f;
	originalData.maxShields = 100.0f;
	originalData.maxEnergy = 150.0f;

	originalData.health = originalData.maxHealth;
	originalData.shields = originalData.maxShields;
	originalData.energy = originalData.maxEnergy;

	originalData.shieldRegen = 20;
	originalData.energyRegen = 15;
	originalData.movementSpeed = 1000;

	//Current Ship data to allow for modifications
	currentData.maxHealth = originalData.maxHealth;
	currentData.maxShields = originalData.maxShields;
	currentData.maxEnergy = originalData.maxEnergy;

	currentData.health = originalData.maxHealth;
	currentData.shields = originalData.maxShields;
	currentData.energy = originalData.maxEnergy;

	currentData.shieldRegen = originalData.shieldRegen;
	currentData.energyRegen = originalData.energyRegen;
	currentData.movementSpeed = originalData.movementSpeed;

	canRegenShields = true;
	canRegenEnergy = true;
	firing = false;
}

void AShip::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);

	// set up gameplay key bindings
	InputComponent->BindAxis(MoveForwardBinding);
	InputComponent->BindAxis(MoveRightBinding);
	InputComponent->BindAxis(FireForwardBinding);
	InputComponent->BindAxis(FireRightBinding);

	InputComponent->BindAction("ActivateAbility", EInputEvent::IE_Released, this, &AShip::ActivateAbility);
	InputComponent->BindAction("WeaponOne", EInputEvent::IE_Released, this, &AShip::WeaponOne);
	InputComponent->BindAction("WeaponTwo", EInputEvent::IE_Released, this, &AShip::WeaponTwo);
	InputComponent->BindAction("WeaponThree", EInputEvent::IE_Released, this, &AShip::WeaponThree);
	InputComponent->BindAction("LMBD", EInputEvent::IE_Released, this, &AShip::StopFire);
	InputComponent->BindAction("LMBD", EInputEvent::IE_Pressed, this, &AShip::StartFire);	
}

void AShip::ActivateAbility() {
	if (abilities.Num() > 0){
		if (!abilities[0]->GetOnCooldown()){
			abilities[0]->SetOnCooldown(GetWorld());
			abilities[0]->Activate(this);
		}
	}
}

void AShip::Regenerate(float DeltaSeconds) {
	if (canRegenShields && GetShield() < GetMaxShield() && GetEnergy() > 0){
		float amount = GetShieldRegen() * (DeltaSeconds / 1);

		if (amount + GetShield() < GetMaxShield()){
			currentData.shields += amount;
			currentData.energy -= amount;
		}
		else {
			currentData.energy -= (GetMaxShield() - GetShield());
			currentData.shields = GetMaxShield();
		}
	}
	else if (canRegenEnergy && GetEnergy() < GetMaxEnergy()){
		float amount = GetEnergyRegen() * (DeltaSeconds / 1);

		if (amount + currentData.energy < GetMaxEnergy()){
			currentData.energy += amount;
		}
		else {
			currentData.energy = GetMaxEnergy();
		}
	}
}

void AShip::Move(float DeltaSeconds){
	// Find movement direction
	const float ForwardValue = GetInputAxisValue(MoveForwardBinding);
	const float RightValue = GetInputAxisValue(MoveRightBinding);
	
	// Calculate movement
	const FVector Movement = GetRootComponent()->GetForwardVector() * ForwardValue * MoveSpeed * DeltaSeconds;

	// If non-zero size, move this actor
	if (ForwardValue != 0)
	{
		FRotator NewRotation = GetRootComponent()->GetComponentRotation();
		FHitResult Hit(1.f);
		RootComponent->MoveComponent(Movement, NewRotation, true, &Hit);

		if (Hit.IsValidBlockingHit())
		{
			const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
			const FVector Deflection = FVector::VectorPlaneProject(Movement, Normal2D) * (1.f - Hit.Time);
			RootComponent->MoveComponent(Deflection, NewRotation, true);
		}
	}
}

/**
 * Begin Actor Interface
 */
void AShip::Tick(float DeltaSeconds)
{
	Regenerate(DeltaSeconds);
	UpdateRotation();
	Move(DeltaSeconds);

	// Create fire direction vector
	const float FireForwardValue = GetInputAxisValue(FireForwardBinding);
	const float FireRightValue = GetInputAxisValue(FireRightBinding);
	const FVector FireDirection = GetRootComponent()->GetComponentRotation().Vector();

	// Try and fire a shot
	FireShot(FireDirection);

	UpdateStats();
}


void AShip::UpdateRotation(){
	FVector mouseLocation, mouseDirection;
	shipController->DeprojectMousePositionToWorld(mouseLocation, mouseDirection);

	FRotator currentCharacterRotation = GetRootComponent()->GetComponentRotation();
	FRotator targetRotation = mouseDirection.Rotation();

	FRotator newRotation = FRotator(currentCharacterRotation.Pitch, targetRotation.Yaw, currentCharacterRotation.Roll);

	GetRootComponent()->SetWorldRotation(newRotation);
}

/**
 * Fire a shot in the specified direction
 */
void AShip::FireShot(FVector FireDirection)
{
	if (firing && currentWeapon != NULL && currentWeapon->CanFire() && GetEnergy() > currentWeapon->GetProjectileData().cost)
	{
		if (currentWeapon->Fire(this, FireDirection)){

			canRegenEnergy = false;
			currentData.energy -= currentWeapon->GetProjectileData().cost;

			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_CanRegenEnergyExpired);
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_CanRegenEnergyExpired, this, &AShip::ResetCanRegenEnergy, 1.5);
		}
	}
}

void AShip::ResetCanFire() {
	currentWeapon->bCanFire = true;
}

USphereComponent* AShip::GetAoeDetection(){
	return aoeDecetion;
}

void AShip::HitByProjectile(ABaseProjectile* damager){
	CalculateDamage(damager->GetDamage());
}

void AShip::CalculateDamage(float damage){
	canRegenShields = false;
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_CanRegenShieldsExpired);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_CanRegenShieldsExpired, this, &AShip::ResetCanRegenShields, 1.5);

	if (GetShield() > 0){
		if (GetShield() - damage > 0){
			currentData.shields -= damage;
		}
		else {
			damage -= GetShield();
			currentData.shields = 0;
			currentData.health -= damage;
		}
	}
	else if (GetHealth() - damage < 0){
		//Destroy();
		//SetActorHiddenInGame(true);
	}
	else {
		currentData.health -= damage;
	}
}

void AShip::ResetCanRegenShields(){
	canRegenShields = true;
}

void AShip::ResetCanRegenEnergy(){
	canRegenEnergy = true;
}

float AShip::GetHealth(){
	return currentData.health;
}

float AShip::GetShield(){
	return currentData.shields;
}

float AShip::GetEnergy(){
	return currentData.energy;
}

void AShip::UpdateStats(){
	location = RootComponent->GetComponentLocation();
	rotation = RootComponent->GetComponentRotation();
}

void AShip::OnRep_EnergyChanged(){

}

void AShip::OnRep_HealthChanged(){

}

void AShip::OnRep_ShieldsChanged(){

}

void AShip::WeaponOne(){
	currentWeapon = weapons[0];
}

void AShip::WeaponThree(){
	currentWeapon = weapons[1];
}

void AShip::WeaponTwo(){
	currentWeapon = weapons[2];
}

void AShip::SetEnergy(float newVal){
	currentData.energy = newVal;
}

void AShip::SetHealth(float newVal){
	currentData.health = newVal;
}

void AShip::SetShield(float newVal){
	currentData.shields = newVal;
}

float AShip::GetMaxEnergy(){
	return currentData.maxEnergy;
}

float AShip::GetMaxHealth(){
	return currentData.maxHealth;
}

float AShip::GetMaxShield(){
	return currentData.maxShields;
}

void AShip::SetMaxEnergy(float newVal){
	currentData.maxEnergy = newVal;
}

void AShip::SetMaxHealth(float newVal){
	currentData.maxHealth = newVal;
}

void AShip::SetMaxShield(float newVal){
	currentData.maxShields = newVal;
}

float AShip::GetEnergyRegen(){
	return currentData.energyRegen;
}

float AShip::GetShieldRegen(){
	return currentData.shieldRegen;
}

void AShip::StartFire(){
	firing = true;
}

void AShip::StopFire(){
	firing = false;
}

void AShip::SetSpeed(float newSpeed){
	MoveSpeed = newSpeed;
	currentData.movementSpeed = newSpeed;
}