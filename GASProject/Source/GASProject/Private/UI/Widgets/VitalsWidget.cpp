#include "UI/Widgets/VitalsWidget.h"

#include "Components/ProgressBar.h"
#include "GAS/Attributes/BaseAttributeSet.h"

void UVitalsWidget::GetTrackedAttributes(TArray<FGameplayAttribute>& OutAttributes) const
{
	Super::GetTrackedAttributes(OutAttributes);
	OutAttributes.AddUnique(UBaseAttributeSet::GetHealthAttribute());
	OutAttributes.AddUnique(UBaseAttributeSet::GetMaxHealthAttribute());
	OutAttributes.AddUnique(UBaseAttributeSet::GetManaAttribute());
	OutAttributes.AddUnique(UBaseAttributeSet::GetMaxManaAttribute());
}

void UVitalsWidget::RefreshBars()
{
	Super::RefreshBars();
	UpdateAttributeBar(HealthBar, UBaseAttributeSet::GetHealthAttribute(),
		UBaseAttributeSet::GetMaxHealthAttribute());
	UpdateAttributeBar(ManaBar, UBaseAttributeSet::GetManaAttribute(),
		UBaseAttributeSet::GetMaxManaAttribute());
}
