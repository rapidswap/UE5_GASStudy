// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/AnimNotify_ComboCancel.h"
#include "Components/SkeletalMeshComponent.h"
#include "Interfaces/ComboAttackInterface.h"

void UAnimNotify_ComboCancel::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    AActor* Owner = MeshComp->GetOwner();
    if (IsValid(Owner) == false)
    {
        return;
    }

    // ComboAttackInterface를 상속 받았으면.
    if (Owner->GetClass()->ImplementsInterface(UComboAttackInterface::StaticClass()))
    {
        // 인터페이스의 순수 가상 함수를 호출.
        // 호출하는 쪽은 Actor가 무슨 타입인지 신경쓸 필용 없이 인터페이스에서 구현해야 할 기능만 호출하면 됨.
        IComboAttackInterface::Execute_RemoveComboAttackBinding(Owner);
    }
}

FString UAnimNotify_ComboCancel::GetNotifyName_Implementation() const
{
    return FString(TEXT("Combo Cancel"));
}
