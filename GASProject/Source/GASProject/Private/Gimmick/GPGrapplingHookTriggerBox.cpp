// Fill out your copyright notice in the Description page of Project Settings.




#include "Gimmick/GPGrapplingHookTriggerBox.h"
#include "Characters/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Components/ShapeComponent.h"
#include "GAS/Tags/GASGameplayTags.h"

AGPGrapplingHookTriggerBox::AGPGrapplingHookTriggerBox(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	OnActorBeginOverlap.AddDynamic(this, &ThisClass::HandleActorBeginOverlap);
	OnActorEndOverlap.AddDynamic(this, &ThisClass::HandleActorEndOverlap);

	// 트리거 충돌 설정.
	if (UShapeComponent* Collision = GetCollisionComponent())
	{
		Collision->SetCollisionProfileName(TEXT("Trigger"));
		Collision->SetGenerateOverlapEvents(true);
	}
}

void AGPGrapplingHookTriggerBox::HandleActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);

	// 유효성 검사.
	if (!PlayerCharacter)
	{
		return;
	}


	if (UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent())
	{
		
		ASC->RemoveLooseGameplayTag(CanGrapplingHookTag);
	
		if (PlayerCharacter->GetGrapplingTargetActor() == GrapplingTarget.Get())
		{
			PlayerCharacter->SetGrapplingTarget(nullptr);
		}
	}
}

void AGPGrapplingHookTriggerBox::HandleActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);

	// 유효성 검사.
	if (!PlayerCharacter)
	{
		return;
	}

	
	if (UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent())
	{
		// 태그 변경 보다 먼저 대상 지정.
		PlayerCharacter->SetGrapplingTarget(GrapplingTarget.Get());

		ASC->AddLooseGameplayTag(CanGrapplingHookTag);
	}
}
