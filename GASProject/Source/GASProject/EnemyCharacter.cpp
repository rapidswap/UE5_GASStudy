// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "BaseAttributeSet.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (BaseAttributeInitEffect)
	{
		// GameplayAbility와 동일하게 원본 형태로 사용하는 것이 아닌 FGameplayEffectSpecHandle구조체로 변환후 ASC 등록 사용.
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle BaseEffectSpecHandle = ASC->MakeOutgoingSpec(BaseAttributeInitEffect, 1, EffectContext);
		if (BaseEffectSpecHandle.IsValid())
		{
			// Handle을 가지고 타겟의 ASC에 Effect 적용. (GA의 Activate 처리).
			// BaseEffectSpecHandle.Data -> TSharedPtr<UGameplayEffectSpec>
			ASC->ApplyGameplayEffectSpecToTarget(*BaseEffectSpecHandle.Data.Get(), ASC);
		}
	}


}

void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ASC->InitAbilityActorInfo(this, this);
}

void AEnemyCharacter::OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	Destroy();
}

UAbilitySystemComponent* AEnemyCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}

void AEnemyCharacter::OnAttack_Implementation()
{
}

void AEnemyCharacter::OnHit_Implementation()
{
	if (OnHitMontage)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(OnHitMontage);
	}
}

void AEnemyCharacter::OnDie_Implementation()
{
	SetActorEnableCollision(false);

	if (OnDieMontage)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(OnDieMontage);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ThisClass::OnDeathMontageEnded);

		// 몽타주가 끝난 다음 호출될 콜백 이벤트에 델리게이트로 등록.
		GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate, OnDieMontage);
	}
}
