// Fill out your copyright notice in the Description page of Project Settings.


#include "PaintBall.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PaintingVolume.h"
#include "PaintingVolumeSubsystem.h"
#include "Player/LyraPlayerState.h"
#include "Teams/LyraTeamSubsystem.h"
#include "Teams/LyraTeamDisplayAsset.h"
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
	
	if (HasAuthority())
	{
		check(TeamMaterial);
		check(ProjectileSphere);
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(TeamMaterial, this);
		if (DynamicMaterial)
		{
			if (APawn* InstigatorActor = GetInstigator())
			{
				if (ALyraPlayerState* LyraPS = InstigatorActor->GetPlayerState<ALyraPlayerState>())
				{
					ULyraTeamSubsystem* LyraTeamSubsystem = GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
					if (LyraTeamSubsystem)
					{
						ULyraTeamDisplayAsset* TeamDisplayAsset =
							LyraTeamSubsystem->GetTeamDisplayAsset(LyraPS->GetTeamId(), LyraPS->GetTeamId());
						if (TeamDisplayAsset)
						{//페인트 볼의 색상을 자기 팀의 색상으로 설정합니다
							DynamicMaterial->SetVectorParameterValue(FName(TEXT("TeamColor")),*TeamDisplayAsset->ColorParameters.Find(FName(TEXT("TeamColor"))));
							ProjectileSphere->SetMaterial(0, DynamicMaterial);
						}
					}
				}
			}
		}
	}
	
	
	
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
		{//Static메시에 대한 충돌처리를 진행합니다
			if (APawn* InstigatorActor = GetInstigator())
			{
				if (ALyraPlayerState* LyraPS = InstigatorActor->GetPlayerState<ALyraPlayerState>())
				{
					ULyraTeamSubsystem* LyraTeamSubsystem = GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
					if (LyraTeamSubsystem)
					{
						ULyraTeamDisplayAsset* TeamDisplayAsset = 
							LyraTeamSubsystem->GetTeamDisplayAsset(LyraPS->GetTeamId(), LyraPS->GetTeamId());
						if (TeamDisplayAsset)
						{
							DrawDebugPoint(GetWorld(), Hit.Location, 5.f, FColor::Red, false, 5.f);
							//서버의 충돌정보를 이용해서 클라이언트에 충돌사실을 알립니다
							UE_LOG(LogTemp, Warning, TEXT("%s"), *Hit.Normal.ToCompactString());
							MulticastRPCPaint(Hit.Location,Hit.Normal,*TeamDisplayAsset->ColorParameters.Find(FName(TEXT("TeamColor"))));
						}
					}
				}
			}
		}
		Destroy();//모든 연산을 수행했으므로 파괴합니다
	}
}

void APaintBall::MulticastRPCPaint_Implementation(FVector Location,FVector PointNormal,FLinearColor Color)
{
	UPaintingVolumeSubsystem* PaintingVolumeSubSystem =
		GetWorld()->GetGameInstance()->GetSubsystem<UPaintingVolumeSubsystem>();
	check(SplatTexture);
	check(PaintingVolumeSubSystem);
	
	if (APaintingVolume* PaintingVolume = PaintingVolumeSubSystem->GetPaintingVolumeInstance(Location))
	{//해당 영역이 속한 PaintingVolume인스턴스를 가져옵니다
		
		DrawDebugPoint(GetWorld(), Location, 10.f, FColor::Green, false, 5.f);
		//그리기 연산을 수행합니다
		
		PaintingVolume->Paint(Location,PointNormal,PaintSize,Color,SplatTexture);
				
	}
}

