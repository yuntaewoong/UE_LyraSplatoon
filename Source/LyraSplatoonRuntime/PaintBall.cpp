// Fill out your copyright notice in the Description page of Project Settings.


#include "PaintBall.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "PaintableComponent.h"
#include "Kismet/GameplayStatics.h"

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
		if (UPaintableComponent* PaintableComp = OtherActor->GetComponentByClass<UPaintableComponent>())
		{//UPaintableComponent가 부착된 액터라면 다시 LineTrace를 수행한 후 대상 Mesh에 Painting적용
			//LineTrace를 다시하는 이유: 충돌한 UV좌표를 얻기위함
			UE_LOG(LogTemp, Log, TEXT("HIT"));
			FVector Start = GetActorLocation();
			FVector End = Start + (FVector::DownVector * 1000.0f);

			FHitResult OutHit;

			FCollisionQueryParams CollisionParams;
			CollisionParams.bTraceComplex = true; //UV좌표를 얻기 위해서 bTraceComplex를 true로 설정
			CollisionParams.bReturnFaceIndex = true;//충돌한 mesh의 face index를 리턴합니다
			CollisionParams.AddIgnoredActor(this);

			bool bHit = GetWorld()->LineTraceSingleByChannel(
				OutHit,
				Start,
				End,
				ECC_Visibility,
				CollisionParams
			);
			if (bHit)
			{
				DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);
				DrawDebugPoint(GetWorld(), OutHit.Location, 10, FColor::Red, false, 1);

				FVector2D UV;
				if (UGameplayStatics::FindCollisionUV(OutHit, 0, UV))
				{
					UE_LOG(LogTemp, Warning, TEXT("UV Found"));
					PaintableComp->PaintCircle(UV, 5);
				}

			}
			else
			{
				DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 1, 0, 1);
			}

		}
		
	}
}

