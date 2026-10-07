#include "UI/Widgets/AttributeWidgetBase.h"

#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"

void UAttributeWidgetBase::SetTargetASC(UAbilitySystemComponent* InASC)
{
	TargetASC = InASC;
	BindASC();
	RefreshBars();
}

UAbilitySystemComponent* UAttributeWidgetBase::GetTargetASC() const
{
	return TargetASC.Get();
}

void UAttributeWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	// WidgetComponent에서 먼저 전달한 ASC를 유지한다.
	BindASC();
	RefreshBars();
}

void UAttributeWidgetBase::NativeDestruct()
{
	UnbindASC();
	Super::NativeDestruct();
}

void UAttributeWidgetBase::GetTrackedAttributes(TArray<FGameplayAttribute>& OutAttributes) const
{
}

void UAttributeWidgetBase::BindASC()
{
	UnbindASC();

	UAbilitySystemComponent* ASC = TargetASC.Get();
	if (!ASC)
	{
		return;
	}

	BoundASC = ASC;
	TArray<FGameplayAttribute> Attributes;
	GetTrackedAttributes(Attributes);

	for (const FGameplayAttribute& Attribute : Attributes)
	{
		if (ASC->HasAttributeSetForAttribute(Attribute))
		{
			BoundAttributes.Add(Attribute);
			AttributeChangeHandles.Add(ASC->GetGameplayAttributeValueChangeDelegate(Attribute)
				.AddUObject(this, &ThisClass::OnAttributeChanged));
		}
	}
}

void UAttributeWidgetBase::UnbindASC()
{
	if (UAbilitySystemComponent* ASC = BoundASC.Get())
	{
		for (int32 Index = 0; Index < BoundAttributes.Num(); ++Index)
		{
			ASC->GetGameplayAttributeValueChangeDelegate(BoundAttributes[Index])
				.Remove(AttributeChangeHandles[Index]);
		}
	}

	BoundAttributes.Reset();
	AttributeChangeHandles.Reset();
	BoundASC.Reset();
}

void UAttributeWidgetBase::OnAttributeChanged(const FOnAttributeChangeData& Data)
{
	RefreshBars();
}

void UAttributeWidgetBase::RefreshBars()
{
}

void UAttributeWidgetBase::UpdateAttributeBar(UProgressBar* Bar, const FGameplayAttribute& Attribute,
	const FGameplayAttribute& MaxAttribute) const
{
	if (!Bar)
	{
		return;
	}

	float Percent = 0.0f;
	if (UAbilitySystemComponent* ASC = TargetASC.Get())
	{
		if (ASC->HasAttributeSetForAttribute(Attribute) && ASC->HasAttributeSetForAttribute(MaxAttribute))
		{
			const float MaxValue = ASC->GetNumericAttribute(MaxAttribute);
			if (MaxValue > 0.0f)
			{
				Percent = FMath::Clamp(ASC->GetNumericAttribute(Attribute) / MaxValue, 0.0f, 1.0f);
			}
		}
	}

	Bar->SetPercent(Percent);
}
