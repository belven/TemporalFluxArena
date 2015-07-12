// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "TemporalFluxArena.h"
#include "Ship.h"
#include "BaseProjectile.h"
#include "Weapon.h"
#include "Teleport.h"
#include "TimerManager.h"
#include "Engine.h"
#include "ShipController.h"
#include "UnrealNetwork.h"

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

	//weapons = *new TArray<UWeapon*>();
	//weapons.Add(UWeapon::CreateWeapon(this, 100, 0.5, 0.1, FVector(90.f, 0.f, 0.f)));
	//weapons.Add(UWeapon::CreateWeapon(this, 100, 0.1f, 0.1f, FVector(300.f, -110, -110)));

	currentWeapon = UWeapon::CreateWeapon(this, 100, 0.5, 0.1, FVector(90.f, 0.f, 0.f));

	abilities = *new TArray<UAbility*>();
	abilities.Add(UTeleport::CreateAbility());
	abilities[0]->SetCooldown(1);
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

	location = RootComponent->GetComponentLocation();
	rotation = RootComponent->GetComponentRotation();
}

void AShip::FireShot(FVector FireDirection)
{
	if (currentWeapon != NULL && currentWeapon->CanFire() && energy > 5)
	{
		if (currentWeapon->Fire(this, FireDirection)){

			canRegenEnergy = false;
			energy -= currentWeapon->GetProjectileData().cost;

			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_CanRegenEnergyExpired);
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_CanRegenEnergyExpired, this, &AShip::ResetCanRegenEnergy, 1.5);
		}
	}
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