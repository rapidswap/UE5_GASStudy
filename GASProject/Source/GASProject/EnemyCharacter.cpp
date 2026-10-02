// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"

AEnemyCharacter::AEnemyCharacter(/*const FObjectInitializer& ObjectInitializer*/ )
	//:Super(ObjectInitializer.SetDefaultSubobjectClass<UEnemyAttributesSet>(TEXT("BaseAttributeSet")))
{
	PrimaryActorTick.bCanEverTick = true;

}

void AEnemyCharacter::OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	Destroy();
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
