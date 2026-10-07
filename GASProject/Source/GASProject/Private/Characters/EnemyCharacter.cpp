// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/EnemyCharacter.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/Widgets/VitalsWidget.h"

AEnemyCharacter::AEnemyCharacter(/*const FObjectInitializer& ObjectInitializer*/ )
	//:Super(ObjectInitializer.SetDefaultSubobjectClass<UEnemyAttributesSet>(TEXT("BaseAttributeSet")))
{
	PrimaryActorTick.bCanEverTick = true;

	VitalsWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("VitalsWidget"));
	VitalsWidgetComponent->SetupAttachment(RootComponent);
	VitalsWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	VitalsWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	VitalsWidgetComponent->SetDrawSize(FVector2D(160.0f, 36.0f));
	VitalsWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
	VitalsWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!VitalsWidgetClass)
	{
		return;
	}

	VitalsWidgetComponent->SetWidgetClass(VitalsWidgetClass);
	VitalsWidgetComponent->InitWidget();

	if (UVitalsWidget* Widget = Cast<UVitalsWidget>(VitalsWidgetComponent->GetUserWidgetObject()))
	{
		Widget->SetTargetASC(ASC);
	}
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
