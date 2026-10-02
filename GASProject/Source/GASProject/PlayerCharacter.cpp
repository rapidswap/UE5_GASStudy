// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GASGameplayTags.h"
#include "PlayerAttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PlayerCharacter)

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UPlayerAttributeSet>(TEXT("BaseAttributeSet")))
{
	// 카메라 붐 생성 및 설정.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent); // 액터의 루트 컴포넌트 하위에 등록.
	CameraBoom->TargetArmLength = 500.0f; // 카메라붐의 길이.
	CameraBoom->bUsePawnControlRotation = true; // 카메라붐이 Pawn의 회전에 맞춰서 같이 회전.

	// 카메라 생성.
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // 카메라는 카메라붐 아래에 등록.
	FollowCamera->bUsePawnControlRotation = false;
}

void APlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveAttackDelegate();
	Super::EndPlay(EndPlayReason);
}

void APlayerCharacter::RemoveComboAttackBinding_Implementation()
{
	RemoveAttackDelegate();
}


void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (InputMappingContext)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		}

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::StopJumping);
		}

		if (AttackAction)
		{
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ThisClass::Attack);
		}
	}
}

void APlayerCharacter::Move(const FInputActionValue& InValue)
{
	if (Controller == nullptr)
	{
		return;
	}

	const FVector2D MovementVector = InValue.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// Get Forward Vector.
	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// Get Right Vector.
	const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// Add Input.
	AddMovementInput(ForwardDir, MovementVector.Y); // 정면.
	AddMovementInput(RightDir, MovementVector.X); // 좌우.
}

void APlayerCharacter::Look(const FInputActionValue& InValue)
{
	if (Controller == nullptr)
	{
		return;
	}

	const FVector2D LookAxisVector = InValue.Get<FVector2D>();
	AddControllerYawInput(LookAxisVector.X); // 좌우 YAW 회전.
	AddControllerPitchInput(LookAxisVector.Y); // 상하 Pitch 회전.
}

void APlayerCharacter::Attack()
{
	if (!IsValid(ASC))
	{
		return;
	}

	if (ASC->HasMatchingGameplayTag(NowAttackingTag))
	{
		// 다음 공격 호출.
		bCallNextAttack = true;
	}
	else
	{
		ComboIndex = 0;
		bCallNextAttack = false;

		if (!AttackAbilityHandles.IsValidIndex(ComboIndex))
		{
			return;
		}

		// TryActivateAbility: 호출하면 내부에서 체크한 후에 Ability를 실행.
		const bool bActivate = ASC->TryActivateAbility(AttackAbilityHandles[ComboIndex]);

		// 공격 Ability가 실행되고 다음 공격 Ability가 있다면.
		if(bActivate&& (ComboIndex+1<AttackAbilityHandles.Num()))
		{
			// 특정 GameplayTag가 등록되었을 때 발동하고 싶은 함수를 Delegate에 등록.
			// Handle을 따로 관리하는 이유는 Delegate를 다 사용했으면 삭제하는 과정이 필요하기 때문.
			ComboDelegateHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(NextAttackTag).AddUObject(this, &ThisClass::StartNextAttack);
		}
	}
}

void APlayerCharacter::StartNextAttack(const FGameplayEventData* InPlayLoad)
{
	if (!IsValid(ASC) || bCallNextAttack == false)
	{
		return;
	}

	if (ComboIndex + 1 < AttackAbilityHandles.Num())
	{
		ComboIndex++;
		bCallNextAttack = false;
		const bool bActivate = ASC->TryActivateAbility(AttackAbilityHandles[ComboIndex]);
		// 실행하지 못했다면 기존 등록했던 Delegate 구독 해제.
		if (!bActivate)
		{
			RemoveAttackDelegate();
		}
	}
}

void APlayerCharacter::RemoveAttackDelegate()
{
	if (ComboDelegateHandle.IsValid())
	{
		if (IsValid(ASC))
		{
			if (auto* ComboCallbacks = ASC->GenericGameplayEventCallbacks.Find(NextAttackTag))
			{
				ComboCallbacks->Remove(ComboDelegateHandle);
			}
		}
		ComboDelegateHandle.Reset();
	}
}
