// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "GPGrapplingHookTriggerBox.generated.h"

/**
 * 
 */
UCLASS()
class GASPROJECT_API AGPGrapplingHookTriggerBox : public ATriggerBox
{
	GENERATED_BODY()

public:
	AGPGrapplingHookTriggerBox(const FObjectInitializer& ObjectInitializer);
protected:
	UFUNCTION()
	void HandleActorEndOverlap(
		AActor* OverlappedActor,
		AActor* OtherActor);

	UFUNCTION()
	void HandleActorBeginOverlap(
		AActor* OverlappedActor,
		AActor* OtherActor
	);

protected:
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Grappling")
	TObjectPtr<AActor> GrapplingTarget;
	
};
