// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/GPPlayerStatWidget.h"
#include "Components/ProgressBar.h"

UGPPlayerStatWidget::UGPPlayerStatWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{

}

void UGPPlayerStatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 이름 값을 이용해 위젯 참조 가져오기.
	StaminaProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("PbStaminaBar")));
	ensure(StaminaProgressBar);

	
}

void UGPPlayerStatWidget::UpdateStatBar(float CurrentStamina)
{
	ensure(MaxStamina > 0.0f);

	if (StaminaProgressBar)
	{
		// 최대 체력 대비 현재 체력의 게이지 설정.
		StaminaProgressBar->SetPercent(CurrentStamina / MaxStamina);
	}
}
