// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "NativeGameplayTags.h"
#include "ComboAttackInterface.h"
#include "CombatActorInterface.h"

#include "PlayerCharacterBase.generated.h"

// 전방 선언.
class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UBaseAttributeSet;
class UGameplayEffect;

// GameplayTag 선언.
GASPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MovingBlockTag);				// 이동 제한 Tag
GASPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(NowAttackingTag);			// 공격 중 Tag
GASPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(NextAttackTag);				// 다음 공격 호출 Tag
GASPROJECT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitTraceTag);				// 공격 판정 호출 Tag

UCLASS()
class GASPROJECT_API APlayerCharacterBase : public ACharacter, public IAbilitySystemInterface,public IComboAttackInterface, public ICombatActorInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 컨트롤러 빙의됐을때 호출되는 콜백.
	virtual void PossessedBy(AController* NewController) override;

	
public:
	// Interface.
	UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	void RemoveComboAttackBinding_Implementation() override;

	void OnAttack_Implementation() override;

	void OnHit_Implementation() override;

	void OnDie_Implementation() override;

	
protected:
	// Input.
	virtual void Move(const FInputActionValue& InValue);

	virtual void Look(const FInputActionValue& InValue);
	
	virtual void Attack();

	virtual void Jump() override;

	virtual void StartNextAttack(const FGameplayEventData* InPlayLoad);

	void RemoveAttackDelegate();

	virtual void HitTrace(const FGameplayEventData* InPlayLoad);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category=Camera)
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
		
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> ASC;

	// 공격 GameplayAbilities.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,Category=GameplayAbilities)
	TArray<TSubclassOf<UGameplayAbility>> AttackAbilities = {};

	TArray<FGameplayAbilitySpecHandle> AttackAbilityHandles = {};

	UPROPERTY()
	TObjectPtr<UBaseAttributeSet> BaseAttributeSet;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="GameplayEffect")
	TSubclassOf<UGameplayEffect> BaseAttributeInitEffect;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=HitTrace)
	float CheckDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HitTrace)
	float SphereRadius = 60.0f;

	int32 ComboIndex = 0;
	bool bCallNextAttack = false;
	FDelegateHandle ComboDelegateHandle = {};

	FDelegateHandle HitTraceDelegateHandle = {};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TEMP")
	TObjectPtr<UAnimMontage> OnHitMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TEMP")
	TObjectPtr<UAnimMontage> OnDieMontage;


};
