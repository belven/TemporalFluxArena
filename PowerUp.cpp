// Fill out your copyright notice in the Description page of Project Settings.

#include "TemporalFluxArena.h"
#include "PowerUp.h"
#include "Ship.h"


// Sets default values
APowerUp::APowerUp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Static reference to the mesh to use for the projectile
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ProjectileMeshAsset(TEXT("/Game/TwinStick/Meshes/TwinStickProjectile.TwinStickProjectile"));

	// Create mesh component for the projectile sphere
	mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh0"));
	mesh->SetStaticMesh(ProjectileMeshAsset.Object);
	mesh->AttachTo(RootComponent);
	mesh->BodyInstance.SetCollisionProfileName("Projectile");
	mesh->OnComponentHit.AddDynamic(this, &APowerUp::OnHit);		// set up a notification for when this component hits something
	RootComponent = mesh;

}

// Called when the game starts or when spawned
void APowerUp::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APowerUp::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}


/**
* Function to handle the projectile hitting something
*/
void APowerUp::OnHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		UClass* otherClass = OtherActor->GetClass();

		if (otherClass->IsChildOf(AShip::StaticClass())){
			AShip* otherShip = Cast<AShip>(OtherActor);
			otherShip->SetEnergy(otherShip->GetMaxEnergy());
			Destroy();
		}
	}
}