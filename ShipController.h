// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "ShipController.generated.h"

/**
 * 
 */
UCLASS()
class TEMPORALFLUXARENA_API AShipController : public APlayerController
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	class AShip* owningShip;
	
	AShipController();
};
