// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/LyraGameplayAbility_FromEquipment.h"
#include "GameplayAbility_SplatoonWeapon.generated.h"

/**
 * 모든 색칠가능한 무기의 발동능력의 부모클래스입니다
 */
UCLASS(Abstract)
class LYRASPLATOONRUNTIME_API UGameplayAbility_SplatoonWeapon : public ULyraGameplayAbility_FromEquipment
{
	GENERATED_BODY()
public:
	UGameplayAbility_SplatoonWeapon();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

private:
	UFUNCTION()
	void TaskCallBack();

protected:
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Animation", Meta = (PrivateAccess = true))
    TObjectPtr<UAnimMontage> FireMontage;
};
