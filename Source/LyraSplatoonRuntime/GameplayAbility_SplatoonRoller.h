// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_SplatoonWeapon.h"
#include "GameplayAbility_SplatoonRoller.generated.h"

/**
 * 색칠가능한 무기의 발사 능력중 중 "롤러"를 사용한 발사능력을 정의하는 클래스입니다
 */
UCLASS(Abstract)
class LYRASPLATOONRUNTIME_API UGameplayAbility_SplatoonRoller : public UGameplayAbility_SplatoonWeapon
{
	GENERATED_BODY()
public:
	UGameplayAbility_SplatoonRoller();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
private:
	UFUNCTION(NetMulticast,Reliable)
	void MulticastRPCPaintRect(FVector Center, float Width, float Height,FLinearColor Color);

private:
	UPROPERTY(EditAnywhere,Category = "Roller")
	TObjectPtr<UTexture> RectTexture;//롤러에 의해 색칠될 텍스처

	UPROPERTY(EditAnywhere,Category = "Roller")
	float RelativeForward = 20;//캐릭터 position으로부터 롤러가 위치한곳까지의 상대거리

	UPROPERTY(EditAnywhere,Category = "Roller")
	float RelativeDown = 70;//캐릭터 position으로부터 롤러가 위치한곳까지의 상대거리

	UPROPERTY(EditAnywhere,Category = "Roller")
	float RollerWidth = 80;//색칠되는 영역의 가로 길이

	UPROPERTY(EditAnywhere,Category = "Roller")
	float RollerHeight = 10;//색칠되는 영역의 세로 길이

	UPROPERTY(EditAnywhere,Category = "Roller")
	float PaintExtra = 10;//추가 색칠크기

	UPROPERTY(EditAnywhere,Category = "Roller")
	int32 NumTrace = 8;//롤러 충돌판정에 사용할 구체의 개수

};
