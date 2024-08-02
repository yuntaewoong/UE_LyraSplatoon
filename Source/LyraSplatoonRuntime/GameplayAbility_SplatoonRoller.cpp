// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_SplatoonRoller.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PaintingVolume.h"
#include "PaintingVolumeSubsystem.h"
#include "Player/LyraPlayerState.h"
#include "Teams/LyraTeamSubsystem.h"
#include "Teams/LyraTeamDisplayAsset.h"


UGameplayAbility_SplatoonRoller::UGameplayAbility_SplatoonRoller()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
}

void UGameplayAbility_SplatoonRoller::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	//부모로직 실행(애니메이션 재생)
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	//
	//바닥 칠하는 로직:
	//       1. 롤러의 끝쪽에서 SphereTrace 여러개 호출
	//       2. 충돌한다면 해당 렌더타겟에 원하는 텍스처로 Paint호출

	if (HasAuthority(&ActivationInfo))
	{
		for (int32 i = 0; i < NumTrace; i++)
		{
			// 충돌 결과를 저장할 변수
			FHitResult HitResult;
			check(ActorInfo->AvatarActor.IsValid());
			FVector RollerPos = ActorInfo->AvatarActor->GetActorLocation() + 
				ActorInfo->AvatarActor->GetActorForwardVector() * RelativeForward +
				ActorInfo->AvatarActor->GetActorUpVector() *(-1) * RelativeDown +
				ActorInfo->AvatarActor->GetActorRightVector() * (-1) * RollerWidth / 2;// 롤러의 시작 위치 계산

			FVector RollerRightDir = ActorInfo->AvatarActor->GetActorRightVector();//롤러의 우측방향 벡터계산

			float SubRollerRadius = (RollerWidth / NumTrace) / 2;
			FVector SubRollerPos = RollerPos + RollerRightDir * (i * SubRollerRadius*2);//충돌처리를 적용할 부분의 위치 계산

			TArray<AActor*> IgnoreActors;
			IgnoreActors.Emplace(ActorInfo->AvatarActor.Get());
			bool bHit = UKismetSystemLibrary::SphereTraceSingle(
				GetWorld(),
				SubRollerPos,  
				SubRollerPos,
				SubRollerRadius,
				ETraceTypeQuery::TraceTypeQuery1,
				false,
				IgnoreActors,  // 무시할 액터 목록
				EDrawDebugTrace::ForOneFrame,  // 디버그 드로잉 옵션
				HitResult,
				true
			);
			if (bHit)
			{//충돌처리 결과 반영
				if (ALyraPlayerState* LyraPS = CastChecked<APawn>(GetAvatarActorFromActorInfo())->GetPlayerState<ALyraPlayerState>())
				{
					ULyraTeamSubsystem* LyraTeamSubsystem = GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
					if (LyraTeamSubsystem)
					{
						ULyraTeamDisplayAsset* TeamDisplayAsset = 
							LyraTeamSubsystem->GetTeamDisplayAsset(LyraPS->GetTeamId(), LyraPS->GetTeamId());
						if (TeamDisplayAsset)
						{
							DrawDebugPoint(GetWorld(), SubRollerPos, 5.f, FColor::Red, false, 1.f);
							//서버의 충돌정보를 이용해서 클라이언트에 충돌사실을 알립니다
							MulticastRPCPaintRect(SubRollerPos, SubRollerRadius*2 + PaintExtra, 
								SubRollerRadius,*TeamDisplayAsset->ColorParameters.Find(FName(TEXT("TeamColor"))));
						}
					}
				}
			}
		}
	}
    
	EndAbility(Handle, ActorInfo, ActivationInfo,true,false);
}

void UGameplayAbility_SplatoonRoller::MulticastRPCPaintRect_Implementation(FVector Center, float Width, float Height, FLinearColor Color)
{
	UPaintingVolumeSubsystem* PaintingVolumeSubSystem =
		GetWorld()->GetGameInstance()->GetSubsystem<UPaintingVolumeSubsystem>();
	check(RectTexture);
	check(PaintingVolumeSubSystem);
	
	if (APaintingVolume* PaintingVolume = PaintingVolumeSubSystem->GetPaintingVolumeInstance(Center))
	{//해당 영역이 속한 PaintingVolume인스턴스를 가져옵니다
		
		DrawDebugPoint(GetWorld(), Center, 10.f, FColor::Green, false, 5.f);
		//그리기 연산을 수행합니다
		PaintingVolume->Paint(Center,Width,Color,RectTexture);
				
	}
}
