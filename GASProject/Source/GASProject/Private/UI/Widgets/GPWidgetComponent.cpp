// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/GPWidgetComponent.h"
#include "UI/Widgets/GPUserWidget.h"

void UGPWidgetComponent::InitWidget()
{
	Super::InitWidget();

	// 앞선 단계에서 위젯 컴포넌트에서 생성한 위젯 객체를 원하는 타입으로 형변환.
	UGPUserWidget* GPUserWidget = Cast<UGPUserWidget>(GetWidget());
	if (GPUserWidget)
	{
		GPUserWidget->SetOwningActor(GetOwner());
	}
}