// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_HitTrace.h"
#include "AbilitySystemComponent.h"
#include "PlayerCharacterBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimNotify_HitTrace)

void UAnimNotify_HitTrace::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp->GetOwner();
	if (IsValid(Owner) == false)
	{
		return;
	}

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, HitTraceTag, FGameplayEventData());
}

FString UAnimNotify_HitTrace::GetNotifyName_Implementation() const
{
	return FString(TEXT("Hit Trace"));
}
