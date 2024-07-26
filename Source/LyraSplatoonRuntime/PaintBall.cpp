// Fill out your copyright notice in the Description page of Project Settings.


#include "PaintBall.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "PaintableComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PaintingVolume.h"
#include "Engine/Canvas.h"
#include "Kismet/KismetRenderingLibrary.h"
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
	//블루프린트에서 정의된 히트 이벤트 호출
	OnStaticMeshHitEvent(OtherActor);

	if(HasAuthority())
	{//서버에서만 Hit판정을 진행합니다 
		if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(OtherComp))
		{//
			if (!APaintingVolume::IsPaintableMesh(StaticMesh))
				return;//페인팅 가능한 StaticMesh가 아니라면 return
			DrawDebugPoint(GetWorld(), Hit.Location, 5.f, FColor::Red, false, 5.f);
			
			if (APaintingVolume* PaintingVolume = APaintingVolume::GetInstance(Hit.Location))
			{//해당 영역이 속한 PaintingVolume인스턴스를 가져옵니다
				DrawDebugPoint(GetWorld(), Hit.Location, 10.f, FColor::Green, false, 5.f);	
				//그리기 연산을 수행합니다
				PaintingVolume->Paint(Hit.Location);
			}

		}
		
	}
}


