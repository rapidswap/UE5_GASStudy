// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "Interfaces/ComboAttackInterface.h"

#include "PlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UPlayerVitalsWidget;
class UWidgetComponent;
struct FGameplayTag;

UCLASS()
class GASPROJECT_API APlayerCharacter : public ACharacterBase, public IComboAttackInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void BeginPlay() override;
	

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void RemoveComboAttackBinding_Implementation() override;

	void SetGrapplingTarget(AActor* TargetActor);

	AActor* GetGrapplingTargetActor() { return CurrentGrapplingTarget.Get(); }

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Move(const FInputActionValue& InValue);
	virtual void Look(const FInputActionValue& InValue);
	virtual void Attack();

	void StartSprint();
	void StopSprint();

	virtual void StartNextAttack(const FGameplayEventData* InPlayLoad);
	void RemoveAttackDelegate();

	void SetupWidgetBarConstruct(TObjectPtr<class UGPWidgetComponent>& WidgetComponent,float Height);

	virtual void SetupCharacterWidget(UGPUserWidget* InUserWidget) override;

	virtual void RefreshStatWidgets() override;

	void RefreshGrapplingPrompt();

	void OnCanGrapplingHookChanged(const FGameplayTag Tag, int32 NewCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Grappling")
	void OnGrapplingPromptShown();

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

	// 위젯 컴포넌트.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widget")
	TObjectPtr<class UGPWidgetComponent> StatBar;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Grappling")
	TObjectPtr<UWidgetComponent> GrapplingPrompt;

	// 그래플링 타겟 액터.
	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentGrapplingTarget;
	
	FDelegateHandle GrapplingTagDelegateHandle;

	// 연속 입력에 따른 콤보 예약은 플레이어 전용.
	int32 ComboIndex = 0;
	bool bCallNextAttack = false;
	FDelegateHandle ComboDelegateHandle = {};
};
