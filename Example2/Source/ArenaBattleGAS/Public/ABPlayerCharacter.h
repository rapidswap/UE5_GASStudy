// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ABBaseCharacter.h"
#include "InputActionValue.h"
#include "ABPlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLEGAS_API AABPlayerCharacter : public AABBaseCharacter
{
	GENERATED_BODY()

public:
	AABPlayerCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UCameraComponent> FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> QuaterMoveAction;

	void QuaterMove(const FInputActionValue& Value);
};
