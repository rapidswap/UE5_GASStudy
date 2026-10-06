// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ActiveGameplayEffectHandle.h"
#include "PlayerSprintAbility.generated.h"


class UCharacterMovementComponent;
class UAbilityTask_WaitAttributeChange;
/**
 * 
 */
UCLASS()
class GASPROJECT_API UPlayerSprintAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPlayerSprintAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;


protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	UFUNCTION(BlueprintImplementableEvent, Category = Sprint)
	void OnSprintStarted();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sprint)
	float SprintSpeed = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Sprint)
	TSubclassOf<UGameplayEffect> StaminaDrainEffect;

private:
	UFUNCTION()
	void OnStaminaDepleted();

	TWeakObjectPtr<UCharacterMovementComponent> SprintMovement;
	
	float OriginalWalkSpeed = 0.0f;

	FActiveGameplayEffectHandle DrainEffectHandle;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitAttributeChange> StaminaTask;


};
