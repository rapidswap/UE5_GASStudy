// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAttributeSet.h"
#include "CombatActorInterface.h"
#include "GameplayEffectExtension.h"

void UBaseAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	}
}

void UBaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));

		if (GetHealth() <= 0.0f)
		{
			AActor* OwnerActor = GetOwningActor();
			// HasAuthority의 경우 실행 주최를 의미하는데 권위 서버가 붙어있는 경우 서버에서만 처리할 수 있도록 하는 체크.
			if (OwnerActor && OwnerActor->HasAuthority())
			{
				// 사망 처리.
				// 여기도 Interface를 사용해서 현재 액터가 어떤 액터인지 알지 못해도 사망 페이즈를 실행 할 수 있도록 구성.
				if (OwnerActor->GetClass()->ImplementsInterface(UCombatActorInterface::StaticClass()))
				{
					// 사망 페이지 트리거.
					ICombatActorInterface::Execute_OnDie(OwnerActor);
				}
			}
		}
	}
}
