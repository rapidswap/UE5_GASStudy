// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/GPHpBarWidget.h"
#include "Components/ProgressBar.h"
#include "Interfaces/GPCharacterWidgetInterface.h"

UGPHpBarWidget::UGPHpBarWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	MaxHp = -1.0f;
}

void UGPHpBarWidget::UpdateStatBar(float CurrentHp)
{
	ensure(MaxHp > 0.0f);

	if (HpProgressBar)
	{
		// 최대 체력 대비 현재 체력의 게이지 설정.
		HpProgressBar->SetPercent(CurrentHp / MaxHp);
	}


}

void UGPHpBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 이름 값을 이용해 위젯 참조 가져오기.
	HpProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("PbHpBar")));
	ensure(HpProgressBar);

	// 인터페이스를 통해서 이 위젯의 함수에 플레이어 스탯의 델리게이트 등록 요청.
	IGPCharacterWidgetInterface* CharacterWidgetInterface = Cast<IGPCharacterWidgetInterface>(OwningActor);
	if (CharacterWidgetInterface)
	{
		CharacterWidgetInterface->SetupCharacterWidget(this);
	}
}
