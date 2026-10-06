// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerVitalsWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "GameFramework/Pawn.h"
#include "BaseAttributeSet.h"
#include "PlayerAttributeSet.h"

namespace
{
	// 현재 값뿐 아니라 최대값이 바뀌어도 막대를 갱신.
	TArray<FGameplayAttribute> GetVitalsAttributes()
	{
		return {
			UBaseAttributeSet::GetHealthAttribute(),
			UBaseAttributeSet::GetMaxHealthAttribute(),
			UPlayerAttributeSet::GetStaminaAttribute(),
			UPlayerAttributeSet::GetMaxStaminaAttribute()
		};
	}
}

void UPlayerVitalsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UnBindASC();

	// 이 위젯을 소유한 플레이어의 캐릭터에서 ASC 가져오기.
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());

	if (!IsValid(ASC))
	{
		return;
	}

	BoundASC = ASC;

	// 속성이 변할 때 호출할 함수 등록.
	for (const FGameplayAttribute& Attribute : GetVitalsAttributes())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &UPlayerVitalsWidget::OnVitalsChanged);
	}

	// 처음 표시할 때도 현재 수치 반영.
	RefreshBars();
} 

void UPlayerVitalsWidget::NativeDestruct()
{
	// 위젯이 사라지면 속성 변경 구독 해제.
	UnBindASC();

	Super::NativeDestruct();
}

void UPlayerVitalsWidget::RefreshBars()
{
	UAbilitySystemComponent* ASC = BoundASC.Get();

	if (!ASC)
	{
		return;
	}

	const float Health = ASC->GetNumericAttribute(UBaseAttributeSet::GetHealthAttribute());
	const float MaxHealth = ASC->GetNumericAttribute(UBaseAttributeSet::GetMaxHealthAttribute());

	const float Stamina = ASC->GetNumericAttribute(UPlayerAttributeSet::GetStaminaAttribute());
	const float MaxStamina = ASC->GetNumericAttribute(UPlayerAttributeSet::GetMaxStaminaAttribute());

	if (HealthBar)
	{
		HealthBar->SetPercent(
			MaxHealth > 0.0f
			? FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f)
			: 0.0f);
	}

	if (StaminaBar)
	{
		StaminaBar->SetPercent(
			MaxStamina > 0.0f
			? FMath::Clamp(Stamina / MaxStamina, 0.0f, 1.0f)
			: 0.0f);
	}

}

void UPlayerVitalsWidget::OnVitalsChanged(const FOnAttributeChangeData& Data)
{
	RefreshBars();
}

void UPlayerVitalsWidget::UnBindASC()
{
	if (UAbilitySystemComponent* ASC = BoundASC.Get())
	{
		for (const FGameplayAttribute& Attribute : GetVitalsAttributes())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(Attribute).RemoveAll(this);
		}
	}

	BoundASC.Reset();
}
