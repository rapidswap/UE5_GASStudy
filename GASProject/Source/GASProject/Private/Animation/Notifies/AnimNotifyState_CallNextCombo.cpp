// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/AnimNotifyState_CallNextCombo.h"
#include "Interfaces/ComboAttackInterface.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GAS/Tags/GASGameplayTags.h"



void UAnimNotifyState_CallNextCombo::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	
	AActor* Owner = MeshComp->GetOwner();

	if (IsValid(Owner) == false)
	{
		return;
	}

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, NextAttackTag, FGameplayEventData());
}
