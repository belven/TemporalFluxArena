// Fill out your copyright notice in the Description page of Project Settings.

#include "TemporalFluxArena.h"
#include "ShieldPowerUp.h"
#include "Ship.h"




/**
* Function to handle the projectile hitting something
*/
void AShieldPowerUp::OnHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		UClass* otherClass = OtherActor->GetClass();

		if (otherClass->IsChildOf(AShip::StaticClass())){
			AShip* otherShip = Cast<AShip>(OtherActor);
			otherShip->SetShield(otherShip->GetMaxShield());
			Destroy();
		}
	}
}