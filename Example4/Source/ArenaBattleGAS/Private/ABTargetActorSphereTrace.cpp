// Fill out your copyright notice in the Description page of Project Settings.


#include "ABTargetActorSphereTrace.h"
#include "Abilities/GameplayAbility.h"

AABTargetActorSphereTrace::AABTargetActorSphereTrace()
{
}

void AABTargetActorSphereTrace::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
}

void AABTargetActorSphereTrace::ConfirmTargetingAndContinue()
{
	check(ShouldProduceTargetData());
	if (IsConfirmTargetingAllowed())
	{
		FHitResult HitResult = GetSphereTraceResult(SourceActor);
		FGameplayAbilityTargetDataHandle DataHandle(new FGameplayAbilityTargetData_SingleTargetHit(HitResult));
		TargetDataReadyDelegate.Broadcast(DataHandle);
	}
}
