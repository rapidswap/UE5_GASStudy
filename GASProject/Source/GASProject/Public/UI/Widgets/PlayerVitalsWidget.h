#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/VitalsWidget.h"
#include "PlayerVitalsWidget.generated.h"

class UProgressBar;

// 공통 체력·마나 위젯에 플레이어의 스태미나 표시를 추가한다.
UCLASS()
class GASPROJECT_API UPlayerVitalsWidget : public UVitalsWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Vitals|Layout")
	void SetPlayerViewportLayout(FVector2D Size, FVector2D Offset);

protected:
	virtual void GetTrackedAttributes(TArray<FGameplayAttribute>& OutAttributes) const override;
	virtual void RefreshBars() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;
};
