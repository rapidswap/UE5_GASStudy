// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "ABTargetActorSphereTrace.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLEGAS_API AABTargetActorSphereTrace : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AABTargetActorSphereTrace();

	virtual void StartTargeting(UGameplayAbility* Ability) override;

	virtual void ConfirmTargetingAndContinue() override;
	
protected:
	UFUNCTION(BlueprintImplementableEvent)
	FHitResult GetSphereTraceResult(AActor* InSourceActor);
};
