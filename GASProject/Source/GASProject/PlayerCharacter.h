// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "ComboAttackInterface.h"

#include "PlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UPlayerVitalsWidget;

UCLASS()
class GASPROJECT_API APlayerCharacter : public ACharacterBase, public IComboAttackInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void RemoveComboAttackBinding_Implementation() override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Move(const FInputActionValue& InValue);
	virtual void Look(const FInputActionValue& InValue);
	virtual void Attack();

	void StartSprint();
	void StopSprint();

	virtual void StartNextAttack(const FGameplayEventData* InPlayLoad);
	void RemoveAttackDelegate();

	void CreatePlayerVitals();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category=GameplayEffect)
	TSubclassOf<UGameplayEffect> StaminaRegenEffect;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category=Input)
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GameplayAbilities)
	TSubclassOf<UGameplayAbility> SprintAbility;

	// ASC에 등록한 스프린트 Ability를 찾는 번호표.
	FGameplayAbilitySpecHandle SprintAbilityHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = UI)
	TSubclassOf<UPlayerVitalsWidget> PlayerVitalsWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UPlayerVitalsWidget> PlayerVitalsWidget;

	// 연속 입력에 따른 콤보 예약은 플레이어 전용.
	int32 ComboIndex = 0;
	bool bCallNextAttack = false;
	FDelegateHandle ComboDelegateHandle = {};
};
