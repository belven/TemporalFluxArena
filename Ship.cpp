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
	DOREPLIFETIME(AShip, health);
	DOREPLIFETIME(AShip, shield);
	DOREPLIFETIME(AShip, energy);
	DOREPLIFETIME(AShip, location);
	DOREPLIFETIME(AShip, rotation);
}

void AShip::PossessedBy(class AController* NewController){
	if (NewController->GetClass()->IsChildOf(AShipController::StaticClass())){
		Cast<AShipController>(NewController)->owningShip = this;
	}
}

// Called when the game starts or when spawned
void AShip::BeginPlay()
{
	Super::BeginPlay();

	// Movement
	MoveSpeed = 1000.0f;

	maxHealth = 100.0f;
	maxShield = 100.0f;
	maxEnergy = 150.0f;

	health = maxHealth;
	shield = maxShield;
	energy = maxEnergy;

	shieldRegen = 20;
	energyRegen = 15;

	canRegenShields = true;
	canRegenEnergy = true;

	FProjectileData data;
	data.cost = 20;
	data.damage = 60;
	data.speed = 1.2;

	weapons = *new TArray<UWeapon*>();

	weapons.Add(UWeapon::CreateWeapon(this, 1, FVector(90.f, 0.f, 0.f)));

	weapons[0]->SetProjectileData(data);

	data.cost = 1;
	data.damage = 5;
	data.speed = 4;

	weapons.Add(UWeapon::CreateWeapon(this, 0.1, FVector(90.f, 0.f, 0.f)));
	weapons[1]->SetProjectileData(data);


	weapons.Add(UWeapon::CreateWeapon(this, 0.5, FVector(90.f, 0.f, 0.f)));
	weapons[2]->SetProjectileData(data);

	data.cost = 10;
	data.damage = 15;
	data.speed = 2;

	currentWeapon = weapons[0];

	abilities = *new TArray<UAbility*>();
	abilities.Add(USpeedBoost::CreateAbility());
	abilities[0]->SetCooldown(3);
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
	if (canRegenShields && shield < maxShield && energy > 0){
		float amount = shieldRegen * (DeltaSeconds / 1);

		if (amount + shield < maxShield){
			shield += amount;
			energy -= amount;
		}
		else {
			energy -= (maxShield - shield);
			shield = maxShield;
		}
	}
	else if (canRegenEnergy && energy < maxEnergy){
		float amount = energyRegen * (DeltaSeconds / 1);

		if (amount + energy < maxEnergy){
			energy += amount;
		}
		else {
			energy = maxEnergy;
		}
	}
}

void AShip::Move(float DeltaSeconds){
	// Find movement direction
	const float ForwardValue = GetInputAxisValue(MoveForwardBinding);
	const float RightValue = GetInputAxisValue(MoveRightBinding);

	// Clamp max size so that (X=1, Y=1) doesn't cause faster movement in diagonal directions
	const FVector MoveDirection = FVector(ForwardValue, RightValue, 0.f).GetClampedToMaxSize(1.0f);

	// Calculate movement
	const FVector Movement = MoveDirection * MoveSpeed * DeltaSeconds;

	// If non-zero size, move this actor
	if (Movement.SizeSquared() > 0.0f)
	{
		const FRotator NewRotation = Movement.Rotation();
		FHitResult Hit(10.f);
		RootComponent->MoveComponent(Movement, NewRotation, true, &Hit);

		//if (Hit.IsValidBlockingHit())
		//{
		const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
		const FVector Deflection = FVector::VectorPlaneProject(Movement, Normal2D) * (1.f - Hit.Time);
		RootComponent->MoveComponent(Deflection, NewRotation, true);
		//}
	}
}

/**
 * Begin Actor Interface
 */
void AShip::Tick(float DeltaSeconds)
{
	Regenerate(DeltaSeconds);
	Move(DeltaSeconds);

	// Create fire direction vector
	const float FireForwardValue = GetInputAxisValue(FireForwardBinding);
	const float FireRightValue = GetInputAxisValue(FireRightBinding);
	const FVector FireDirection = FVector(FireForwardValue, FireRightValue, 0.f);

	// Try and fire a shot
	FireShot(FireDirection);

	UpdateStats();
}

/**
 * Fire a shot in the specified direction
 */
void AShip::FireShot(FVector FireDirection)
{
	if (currentWeapon != NULL && currentWeapon->CanFire() && energy > currentWeapon->GetProjectileData().cost)
	{
		if (currentWeapon->Fire(this, FireDirection)){

			canRegenEnergy = false;
			energy -= currentWeapon->GetProjectileData().cost;

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

	if (shield > 0){
		if (shield - damage > 0){
			shield -= damage;
		}
		else {
			damage -= shield;
			shield = 0;
			health -= damage;
		}
	}
	else if (health - damage < 0){
		//Destroy();
		//SetActorHiddenInGame(true);
	}
	else {
		health -= damage;
	}
}

void AShip::ResetCanRegenShields(){
	canRegenShields = true;
}

void AShip::ResetCanRegenEnergy(){
	canRegenEnergy = true;
}


float AShip::GetHealth(){
	return health;
}

float AShip::GetShield(){
	return shield;
}

float AShip::GetEnergy(){
	return energy;
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

	energy = newVal;
}


void AShip::SetHealth(float newVal){

	health = newVal;
}


void AShip::SetShield(float newVal){

	shield = newVal;
}


float AShip::GetMaxEnergy(){

	return maxEnergy;
}


float AShip::GetMaxHealth(){

	return maxHealth;
}


float AShip::GetMaxShield(){

	return maxShield;
}


void AShip::SetMaxEnergy(float newVal){

	maxEnergy = newVal;
}


void AShip::SetMaxHealth(float newVal){

	maxHealth = newVal;
}


void AShip::SetMaxShield(float newVal){

	maxShield = newVal;
}