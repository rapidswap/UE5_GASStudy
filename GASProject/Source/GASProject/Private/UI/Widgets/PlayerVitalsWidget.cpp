#include "UI/Widgets/PlayerVitalsWidget.h"

#include "Blueprint/GameViewportSubsystem.h"
#include "Components/ProgressBar.h"
#include "GAS/Attributes/PlayerAttributeSet.h"

void UPlayerVitalsWidget::SetPlayerViewportLayout(FVector2D Size, FVector2D Offset)
{
	if (UGameViewportSubsystem* ViewportSubsystem = UGameViewportSubsystem::Get())
	{
		// 개별 SetPosition/SetDesiredSize 호출은 앵커를 초기화하므로 한 슬롯에 모아 적용한다.
		FGameViewportWidgetSlot ViewportSlot;
		ViewportSlot.Anchors = FAnchors(0.0f, 1.0f);
		ViewportSlot.Alignment = FVector2D(0.0f, 1.0f);
		ViewportSlot.Offsets = FMargin(Offset.X, Offset.Y, Size.X, Size.Y);
		ViewportSubsystem->SetWidgetSlot(this, ViewportSlot);
	}
}

void UPlayerVitalsWidget::GetTrackedAttributes(TArray<FGameplayAttribute>& OutAttributes) const
{
	Super::GetTrackedAttributes(OutAttributes);
	OutAttributes.AddUnique(UPlayerAttributeSet::GetStaminaAttribute());
	OutAttributes.AddUnique(UPlayerAttributeSet::GetMaxStaminaAttribute());
}

void UPlayerVitalsWidget::RefreshBars()
{
	Super::RefreshBars();
	UpdateAttributeBar(StaminaBar, UPlayerAttributeSet::GetStaminaAttribute(),
		UPlayerAttributeSet::GetMaxStaminaAttribute());
}
