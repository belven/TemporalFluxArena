// Fill out your copyright notice in the Description page of Project Settings.

#include "TemporalFluxArena.h"
#include "Weapon.h"
#include "Ship.h"
#include "BaseProjectile.h"

UWeapon::UWeapon() {
	// Cache our sound effect
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/TwinStick/Audio/TwinStickFire.TwinStickFire"));
	fireSound = FireAudio.Object;

	// Weapon
	offset = FVector(90.f, 0.f, 0.f);
	weaponFireRate = 0.5f;
	bCanFire = true;

	projectileData.damage = 20;
	projectileData.cost = 1;
	projectileData.speed = 4000;
}

bool UWeapon::Fire(AShip* owner, FVector FireDirection) {
	// If we are pressing fire stick in a direction
	if (FireDirection.SizeSquared() > 0.0f)
	{
		const FRotator FireRotation = FireDirection.Rotation();

		// Spawn projectile at an offset from this pawn
		const FVector SpawnLocation = owner->GetActorLocation() + FireRotation.RotateVector(GetOffset());

		UWorld* const World = owner->GetWorld();
		if (World != NULL)
		{
			// spawn the projectile
			ABaseProjectile* projectile = World->SpawnActor<ABaseProjectile>(SpawnLocation, FireRotation);
			projectile->SetDamage(projectileData.damage);
			projectile->SetSpeed(owner->GetMoveSpeed() * projectileData.speed);

			Fired(World);

			// try and play the sound if specified
			if (fireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(owner, fireSound, owner->GetActorLocation());
			}
			return true;
		}

	}
	return false;
}

void UWeapon::Fired(UWorld* world){
	if (world != nullptr && bCanFire) {
		bCanFire = false;
		world->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &UWeapon::ResetCanFire, weaponFireRate > 0 ? weaponFireRate : 0.1);
	}
}

UWeapon* UWeapon::CreateWeapon(AActor* other, float fireRate, FVector offset, FProjectileData data) {
	UWeapon* newWeapon = NewObject<UWeapon>(UWeapon::StaticClass());
	newWeapon->SetFireRate(fireRate);
	newWeapon->offset = offset;
	newWeapon->projectileData = data;
	return newWeapon;
}

void UWeapon::ResetCanFire() {
	bCanFire = true;
}

float UWeapon::GetFireRate() {
	return weaponFireRate;
}

void UWeapon::SetFireRate(float newRate){
	weaponFireRate = newRate;
}

bool UWeapon::CanFire() {
	return bCanFire;
}

FProjectileData UWeapon::GetProjectileData(){
	return projectileData;
}

void UWeapon::SetProjectileData(FProjectileData newVal){
	projectileData = newVal;
}