// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PlayMontageAndWait.generated.h"

/**
 * 
 */
UCLASS()
class GASPROJECT_API UPlayMontageAndWait : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPlayMontageAndWait();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintNativeEvent)
	void OnMontageCompleted();
	virtual void OnMontageCompleted_Implementation();

	UFUNCTION(BlueprintNativeEvent)
	void OnMontageInterrupted();
	virtual void OnMontageInterrupted_Implementation();

	UFUNCTION(BlueprintNativeEvent)
	void OnMontageCancelled();
	virtual void OnMontageCancelled_Implementation();

	UFUNCTION(BlueprintNativeEvent)
	void OnMontageBlendOut();
	virtual void OnMontageBlendOut_Implementation();

protected:
	// Ability 실행 시에 재생할 AnimMontage.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Animation)
	TObjectPtr<UAnimMontage> PlayMontage;
	
};
