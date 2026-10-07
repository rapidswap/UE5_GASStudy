#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/AttributeWidgetBase.h"
#include "VitalsWidget.generated.h"

class UProgressBar;

// 모든 캐릭터가 공유하는 체력·마나 표시.
UCLASS()
class GASPROJECT_API UVitalsWidget : public UAttributeWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void GetTrackedAttributes(TArray<FGameplayAttribute>& OutAttributes) const override;
	virtual void RefreshBars() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	// 체력만 표시하는 Enemy 레이아웃도 사용할 수 있다.
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ManaBar;
};
