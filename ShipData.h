#pragma once
#include "ShipData.generated.h"

USTRUCT(BlueprintType)
struct FShipData
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float energy = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float health = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float maxEnergy = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float maxHealth = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float maxShields = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float shields = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float energyRegen = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float shieldRegen = 100;
	float movementSpeed = 100;
};