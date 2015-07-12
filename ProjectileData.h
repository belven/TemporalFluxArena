#pragma once
#include "ProjectileData.generated.h"

USTRUCT(BlueprintType)
struct FProjectileData
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
		float damage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		float speed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
		float cost = 0;

};