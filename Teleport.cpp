// Fill out your copyright notice in the Description page of Project Settings.

#include "TemporalFluxArena.h"
#include "Teleport.h"
#include "Ship.h"




void UTeleport::Activate(AShip* owner){
	// Clamp max size so that (X=1, Y=1) doesn't cause faster movement in diagonal directions
	const FVector MoveDirection = owner->GetActorRotation().Vector().GetClampedToMaxSize(1.0f);

	// Calculate movement
	const FVector Movement = MoveDirection * 1200;

	// If non-zero size, move this actor
	if (Movement.SizeSquared() > 0.0f)
	{
		const FRotator NewRotation = Movement.Rotation();
		FHitResult Hit(1.f);
		owner->GetRootComponent()->MoveComponent(Movement, NewRotation, true, &Hit);

		if (Hit.IsValidBlockingHit())
		{
			const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
			const FVector Deflection = FVector::VectorPlaneProject(Movement, Normal2D) * (1.f - Hit.Time);
			owner->GetRootComponent()->MoveComponent(Deflection, NewRotation, true);
			owner->UpdateStats();
		}
	}
}

UTeleport* UTeleport::CreateAbility(){
	return NewObject<UTeleport>(UTeleport::StaticClass());
}