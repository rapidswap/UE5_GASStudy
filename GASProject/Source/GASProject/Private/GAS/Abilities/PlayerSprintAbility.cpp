// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/PlayerSprintAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitAttributeChange.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayEffect.h"
#include "GAS/Tags/GASGameplayTags.h"
#include "GAS/Attributes/PlayerAttributeSet.h"
#include "UObject/ConstructorHelpers.h"

UPlayerSprintAbility::UPlayerSprintAbility()
{
	// 캐릭터별로 속도와 이펙트 핸들을 보관할 인스턴스 생성.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Ability 실행 중 ASC에 붙고, 종료동료되면 제거되는 태그.
	ActivationOwnedTags.AddTag(SprintingTag);
	ActivationOwnedTags.AddTag(StaminaRegenBlockedTag);

	// 공격 중이거나 이동이 차단된 상태에서는 시작 금지.
	ActivationBlockedTags.AddTag(NowAttackingTag);
	ActivationBlockedTags.AddTag(MovingBlockTag);

	// 기존에 만든 스테미나 소모 이펙트를 기본값으로 연결.
	//static ConstructorHelpers::FClassFinder<UGameplayEffect> DrainEffectClass(
	//	TEXT("/Game/ThirdPerson/GameEffects/Effects/MoveEffects/BPGE_SprintStamina"));

	//if (DrainEffectClass.Succeeded())
	//{
	//	StaminaDrainEffect = DrainEffectClass.Class;
	//}
}

bool UPlayerSprintAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (!ActorInfo || !StaminaDrainEffect)
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	const ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());

	const UPlayerAttributeSet* Attributes = ASC ? ASC->GetSet<UPlayerAttributeSet>() : nullptr;

	// 스테미나가 있어야 시작 가능.
	return Character && Character->GetCharacterMovement() && Attributes && Attributes->GetStamina() > 0.0f
		&& !Character->GetLastMovementInputVector().IsNearlyZero() && Character->GetVelocity().Size2D() > 10.0f;
}

void UPlayerSprintAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;

	ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;

	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;

	if (!ASC || !Movement || !StaminaDrainEffect)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 기존 속도를 저장하고 스프린트 속도로 변경.
	OriginalWalkSpeed = Movement->MaxWalkSpeed;
	SprintMovement = Movement;
	Movement->MaxWalkSpeed = SprintSpeed;
	FGameplayEffectSpecHandle DrainSpec = MakeOutgoingGameplayEffectSpec(StaminaDrainEffect, GetAbilityLevel());

	if (!DrainSpec.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 자식 소모 이펙트 적용, 종료 시 제거할 수 있도록 핸들 저장.
	DrainEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, DrainSpec);

	if (!DrainEffectHandle.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}




	// 스테미나가 0이하로 변하면 종료 함수를 호출.
	StaminaTask =
		UAbilityTask_WaitAttributeChange::WaitForAttributeChangeWithComparison(
			this,
			UPlayerAttributeSet::GetStaminaAttribute(),
			FGameplayTag(),
			FGameplayTag(),
			EWaitAttributeChangeComparison::LessThanOrEqualTo,
			0.0f,
			true
		);

	StaminaTask->OnChange.AddDynamic(this, &UPlayerSprintAbility::OnStaminaDepleted);

	StaminaTask->ReadyForActivation();

	// 이펙트가 적용 즉시 스테미나를 소모한느 설정 처리.
	if (ASC->GetNumericAttribute(UPlayerAttributeSet::GetStaminaAttribute()) <= 0.0f)
	{
		OnStaminaDepleted();
		return;
	}

	OnSprintStarted();
}

void UPlayerSprintAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!IsEndAbilityValid(Handle, ActorInfo))
	{
		return;
	}

	// GAS 내부 잠금 중에는 정리까지 함께 지연.
	if (ScopeLockCount > 0)
	{
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(
			this,
			&UPlayerSprintAbility::EndAbility,
			Handle,
			ActorInfo,
			ActivationInfo,
			bReplicateEndAbility,
			bWasCancelled
		));

		return;
	}

	// 스테미나 감시 종료.
	if (StaminaTask)
	{
		StaminaTask->EndTask();
		StaminaTask = nullptr;
	}

	// 스테미나 소모 이펙트 제거.
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;

	if (ASC && DrainEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(DrainEffectHandle);
	}

	DrainEffectHandle.Invalidate();

	// 원래 이동 속도로 복구.
	if (UCharacterMovementComponent* Movement = SprintMovement.Get())
	{
		Movement->MaxWalkSpeed = OriginalWalkSpeed;
	}

	SprintMovement.Reset();

	// GAS가 활성 상태와 소유 태그를 정리.
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}


void UPlayerSprintAbility::OnStaminaDepleted()
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), CurrentActivationInfo, true, false);
	}
}
