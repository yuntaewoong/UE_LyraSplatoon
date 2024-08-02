// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_SplatoonWeapon.h"
#include "Abilities\Tasks\AbilityTask_PlayMontageAndWait.h"

UGameplayAbility_SplatoonWeapon::UGameplayAbility_SplatoonWeapon()
{
	
	
}

void UGameplayAbility_SplatoonWeapon::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	CommitAbility(Handle, ActorInfo, ActivationInfo);//설정된 자원을 소모합니다
	check(FireMontage);
	UAbilityTask_PlayMontageAndWait* AbilityTaskPlayMontageAndWait = 
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, FName(TEXT("NONE")), FireMontage);
	if (AbilityTaskPlayMontageAndWait)
	{
		AbilityTaskPlayMontageAndWait->OnCancelled.AddDynamic(this, &UGameplayAbility_SplatoonWeapon::TaskCallBack);
		AbilityTaskPlayMontageAndWait->OnInterrupted.AddDynamic(this, &UGameplayAbility_SplatoonWeapon::TaskCallBack);
		AbilityTaskPlayMontageAndWait->OnCompleted.AddDynamic(this, &UGameplayAbility_SplatoonWeapon::TaskCallBack);
	}
	AbilityTaskPlayMontageAndWait->Activate();

}

void UGameplayAbility_SplatoonWeapon::TaskCallBack()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
