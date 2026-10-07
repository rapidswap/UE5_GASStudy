#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "AttributeWidgetBase.generated.h"

class UAbilitySystemComponent;
class UProgressBar;
struct FOnAttributeChangeData;

// 표시 대상 ASC의 지정, 속성 구독과 수명 정리를 담당하는 위젯 베이스.
UCLASS(Abstract)
class GASPROJECT_API UAttributeWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Vitals")
	void SetTargetASC(UAbilitySystemComponent* InASC);

	UFUNCTION(BlueprintPure, Category = "Vitals")
	UAbilitySystemComponent* GetTargetASC() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void GetTrackedAttributes(TArray<FGameplayAttribute>& OutAttributes) const;
	virtual void RefreshBars();

	void UpdateAttributeBar(UProgressBar* Bar, const FGameplayAttribute& Attribute,
		const FGameplayAttribute& MaxAttribute) const;

private:
	void BindASC();
	void UnbindASC();
	void OnAttributeChanged(const FOnAttributeChangeData& Data);

	TWeakObjectPtr<UAbilitySystemComponent> TargetASC;
	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
	TArray<FGameplayAttribute> BoundAttributes;
	TArray<FDelegateHandle> AttributeChangeHandles;
};
