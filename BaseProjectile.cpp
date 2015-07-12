// Copyright 1998-2015 Epic Games, Inc. All Rights Reserve

#include "TemporalFluxArena.h"
#include "BaseProjectile.h"
#include "Ship.h"
#include "GameFramework/ProjectileMovementComponent.h"

ABaseProjectile::ABaseProjectile()
{
	speed = 3000.0f;

	// Static reference to the mesh to use for the projectile
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ProjectileMeshAsset(TEXT("/Game/TwinStick/Meshes/TwinStickProjectile.TwinStickProjectile"));

	// Create mesh component for the projectile sphere
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh0"));
	ProjectileMesh->SetStaticMesh(ProjectileMeshAsset.Object);
	ProjectileMesh->AttachTo(RootComponent);
	ProjectileMesh->BodyInstance.SetCollisionProfileName("Projectile");
	ProjectileMesh->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);		// set up a notification for when this component hits something
	RootComponent = ProjectileMesh;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement0"));
	ProjectileMovement->UpdatedComponent = ProjectileMesh;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f; // No gravity

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;

	damage = 10;
}

// Called when the game starts or when spawned
void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
}

/**
 * Function to handle the projectile hitting something
 */
void ABaseProjectile::OnHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		UClass* otherClass = OtherActor->GetClass();
		/*if (OtherComp->IsSimulatingPhysics()) {
			OtherComp->AddImpulseAtLocation(GetVelocity() * 20.0f, GetActorLocation());
			}*/

		if (otherClass->IsChildOf(AShip::StaticClass())){
			AShip* otherShip = Cast<AShip>(OtherActor);
			otherShip->HitByProjectile(this);
			Destroy();
		}
		else if (!otherClass->IsChildOf(ABaseProjectile::StaticClass())
			&& !otherClass->IsChildOf(USphereComponent::StaticClass())
			&& !otherClass->IsChildOf(UStaticMeshComponent::StaticClass())){
			Destroy();
		}
	}
}

ABaseProjectile* ABaseProjectile::CreateProjectile(AActor* other, float damage, float speed, int size){
	ABaseProjectile* newProjectile = other->GetWorld()->SpawnActor<ABaseProjectile>(other->GetActorLocation(), other->GetActorRotation());
	newProjectile->SetDamage(damage);
	newProjectile->size = size;
	newProjectile->speed = speed;
	return newProjectile;
}

float ABaseProjectile::GetDamage(){
	return damage;
}

void ABaseProjectile::SetDamage(float newDamage){
	damage = newDamage;
}


float ABaseProjectile::GetSpeed(){
	return speed;
}


void ABaseProjectile::SetSpeed(float newVal){
	speed = newVal;
	ProjectileMovement->InitialSpeed = speed;
	ProjectileMovement->MaxSpeed = speed;
}