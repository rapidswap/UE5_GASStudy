// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "Interfaces/CombatActorInterface.h"

#include "CharacterBase.generated.h"

class UBaseAttributeSet;
class UGameplayAbility;
class UGameplayEffect;
class UAnimMontage;
struct FGameplayEventData;

UCLASS()
class GASPROJECT_API ACharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatActorInterface
{
	GENERATED_BODY()

public:
	ACharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PossessedBy(AController* NewController) override;

	// 플레이어와 에너미가 공통으로 사용하는 GAS/전투 인터페이스.
	UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	void OnAttack_Implementation() override;
	void OnHit_Implementation() override;
	void OnDie_Implementation() override;

protected:
	virtual void BeginPlay() override;

	virtual void HitTrace(const FGameplayEventData* InPlayLoad);

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> ASC;

	// Ability 등록은 공통으로 처리하고, 실행 방식은 각 캐릭터가 결정.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = GameplayAbilities)
	TArray<TSubclassOf<UGameplayAbility>> AttackAbilities = {};

	TArray<FGameplayAbilitySpecHandle> AttackAbilityHandles = {};

	UPROPERTY()
	TObjectPtr<UBaseAttributeSet> BaseAttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameplayEffect")
	TSubclassOf<UGameplayEffect> BaseAttributeInitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HitTrace)
	float CheckDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HitTrace)
	float SphereRadius = 60.0f;

	FDelegateHandle HitTraceDelegateHandle = {};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TEMP")
	TObjectPtr<UAnimMontage> OnHitMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TEMP")
	TObjectPtr<UAnimMontage> OnDieMontage;
};
