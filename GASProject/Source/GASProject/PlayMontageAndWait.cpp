// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UPlayMontageAndWait::UPlayMontageAndWait()
{
}

void UPlayMontageAndWait::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	check(PlayMontage);

	// Ability가 실행될 수 있는지 확인.
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		// 아니면 종료.
		EndAbility(Handle, ActorInfo, ActivationInfo,true,true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		PlayMontage,
		1.0f
	);

	check(Task);

	// Task가 비동기 방식이라서 Task가 특정 상황에서 종료되었을 때 처리를 델리게이트로 바인딩 해야됨.
	Task->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	Task->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
	Task->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	Task->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageBlendOut);

	Task->ReadyForActivation();

	if (bHasBlueprintActivate)
	{
		// BlueprintImplementableEvent로 선언된 ActivatedAbility
		// BP 클래스에서 재정의된 ActivateAbility 함수 호출.
		K2_ActivateAbility();
	}
	
}
void UPlayMontageAndWait::OnMontageCompleted_Implementation()
{
	EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), CurrentActivationInfo, true, false);
}

void UPlayMontageAndWait::OnMontageInterrupted_Implementation()
{
	EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), CurrentActivationInfo, true, true);
}

void UPlayMontageAndWait::OnMontageCancelled_Implementation()
{
	EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), CurrentActivationInfo, true, true);
}

void UPlayMontageAndWait::OnMontageBlendOut_Implementation()
{
	EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), CurrentActivationInfo, true, false);
}
