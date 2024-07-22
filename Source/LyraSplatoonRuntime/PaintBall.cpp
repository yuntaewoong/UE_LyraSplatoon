// Fill out your copyright notice in the Description page of Project Settings.


#include "PaintBall.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"

APaintBall::APaintBall()
{
    bReplicates = true;
	SetReplicateMovement(true);
	
	PrimaryActorTick.bCanEverTick = true;
	

	ProjectileSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Sphere"));
	RootComponent = ProjectileSphere;
	
	ProjectileSphere->SetNotifyRigidBodyCollision(true);
	ProjectileSphere->OnComponentHit.AddDynamic(this, &APaintBall::OnStaticMeshHit);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
}

void APaintBall::BeginPlay()
{
	Super::BeginPlay();
	
}

void APaintBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APaintBall::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void APaintBall::OnStaticMeshHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, 
	const FHitResult& Hit
)
{
	OnStaticMeshHitEvent(OtherActor);
}

