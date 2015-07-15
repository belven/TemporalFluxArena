// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PowerUp.h"
#include "HealthPowerUp.generated.h"

/**
 *
 */
UCLASS()
class TEMPORALFLUXARENA_API AHealthPowerUp : public APowerUp
{
	GENERATED_BODY()

public:
	/** Function to handle the projectile hitting something */
	UFUNCTION()
		void OnHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


};
