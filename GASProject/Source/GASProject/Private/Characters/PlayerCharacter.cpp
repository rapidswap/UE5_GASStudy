// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/PlayerCharacter.h"
#include "InputActionValue.h"
#include "GAS/Tags/GASGameplayTags.h"
#include "GAS/Attributes/PlayerAttributeSet.h"
#include "UI/Widgets/GPWidgetComponent.h"
#include "UI/Widgets/GPPlayerStatWidget.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "AbilitySystemComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "GameplayEffect.h"
#include "InputAction.h"
#include "Components/WidgetComponent.h"


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

	SetupWidgetBarConstruct(StatBar, 200.0f);

	// 그래플링 UI 설정.
	GrapplingPrompt = CreateDefaultSubobject<UWidgetComponent>(TEXT("GrapplingPrompt"));
	GrapplingPrompt->SetupAttachment(GetRootComponent());
	GrapplingPrompt->SetWidgetSpace(EWidgetSpace::Screen);
	GrapplingPrompt->SetDrawSize(FVector2D(100.0f, 40.0f));
	GrapplingPrompt->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GrapplingPrompt->SetVisibility(false);



	// 소모 이펙트가 설정된 스프린트 Ability BP를 에디터에서 지정한다.

	//static ConstructorHelpers::FObjectFinder<UInputAction> SprintInput
	//(TEXT("/Game/Input/Actions/IA_Sprint.IA_Sprint"));

	//if (SprintInput.Succeeded())
	//{
	//	SprintAction = SprintInput.Object;
	//}

	//static ConstructorHelpers::FClassFinder<UPlayerVitalsWidget> VitalsClass(
	//	TEXT("/Game/UI/WBP_PlayerVitals"));

	//if (VitalsClass.Succeeded())
	//{
	//	PlayerVitalsWidgetClass = VitalsClass.Class;
	//}

}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle RegenSpec = ASC->MakeOutgoingSpec(StaminaRegenEffect, 1.0f, Context);

	if (RegenSpec.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*RegenSpec.Data.Get());
	}

	if (SprintAbility)
	{
		FGameplayAbilitySpec SprintSpec(
			SprintAbility,
			1,
			INDEX_NONE,
			this
		);

		SprintAbilityHandle = ASC->GiveAbility(SprintSpec);
	}

	GrapplingTagDelegateHandle =
		ASC->RegisterGameplayTagEvent(CanGrapplingHookTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnCanGrapplingHookChanged);

	RefreshGrapplingPrompt();
}


void APlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveAttackDelegate();

	ASC->UnregisterGameplayTagEvent(
		GrapplingTagDelegateHandle,
		CanGrapplingHookTag,
		EGameplayTagEventType::NewOrRemoved
	);


	Super::EndPlay(EndPlayReason);
}

void APlayerCharacter::RemoveComboAttackBinding_Implementation()
{
	RemoveAttackDelegate();
}

void APlayerCharacter::SetGrapplingTarget(AActor* TargetActor)
{
	// UI를 대상에게 붙임.
	CurrentGrapplingTarget = TargetActor;
	RefreshGrapplingPrompt();
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


		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(
				SprintAction,
				ETriggerEvent::Started,
				this,
				&ThisClass::StartSprint);

			EnhancedInputComponent->BindAction(
				SprintAction,
				ETriggerEvent::Completed,
				this,
				&ThisClass::StopSprint);

			EnhancedInputComponent->BindAction(
				SprintAction,
				ETriggerEvent::Canceled,
				this,
				&ThisClass::StopSprint);
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
		if(bActivate && (ComboIndex+1<AttackAbilityHandles.Num()))
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

void APlayerCharacter::SetupWidgetBarConstruct(TObjectPtr<UGPWidgetComponent>& WidgetComponent, float Height)
{
	WidgetComponent = CreateDefaultSubobject<UGPWidgetComponent>(TEXT("StatBar"));

	// 위젯 컴포넌트는 씬 컴포넌트이기 때문에 계층 설정.
	WidgetComponent->SetupAttachment(GetMesh());

	// 캐릭터 머리 위에 보일 수 있도록 위치 조정.
	WidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, Height));

	// 위젯 설정.
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);

	// UI가 그려질 크기 설정.
	WidgetComponent->SetDrawSize(FVector2D(150.0f, 15.0f));

	// 콜리전 끄기.
	WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APlayerCharacter::SetupCharacterWidget(UGPUserWidget* InUserWidget)
{
	
	Cast<UPlayerAttributeSet>(BaseAttributeSet)->OnStatChanged.AddUObject(this, &ThisClass::RefreshStatWidgets);

}

void APlayerCharacter::RefreshStatWidgets()
{
	//Super::RefreshStatWidgets();

	if (!StatBar)
	{
		return;
	}

	UGPPlayerStatWidget* StaminaBarWidget = Cast<UGPPlayerStatWidget>(StatBar->GetWidget());
	

	if (StaminaBarWidget)
	{
		StaminaBarWidget->SetMaxStamina(Cast<UPlayerAttributeSet>(BaseAttributeSet)->GetMaxStamina());
		StaminaBarWidget->UpdateStatBar(Cast<UPlayerAttributeSet>(BaseAttributeSet)->GetStamina());

	}
}

void APlayerCharacter::RefreshGrapplingPrompt()
{
	// 그래플링 프롬프트 리프레시 설정 -> 태그가 들어올때마다 UI 표시를 위해.
	if (!GrapplingPrompt)
	{
		return;
	}

	// 그래플링 할 타겟.
	AActor* Target = CurrentGrapplingTarget.Get();

	APlayerController* PC = Cast<APlayerController>(GetController());

	const bool bShow =
		ASC &&
		ASC->HasMatchingGameplayTag(CanGrapplingHookTag) &&
		Target &&
		Target->GetRootComponent() &&
		PC &&
		PC->GetLocalPlayer();

	// 하나라도 조건이 맞지 않는다면 UI는 보이지 않음.
	if (!bShow)
	{
		GrapplingPrompt->SetVisibility(false);
		return;
	}

	// 이 플레이어의 화면에만 표시.
	GrapplingPrompt->SetOwnerPlayer(PC->GetLocalPlayer());

	// UI 위치를 대상 액터에 붙이기.
	if (GrapplingPrompt->GetAttachParent() != Target->GetRootComponent())
	{
		GrapplingPrompt->AttachToComponent(Target->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	GrapplingPrompt->SetRelativeLocation(FVector( 0.0f, 0.0f, 100.0f));

	const bool bWasVisible = GrapplingPrompt->IsVisible();
	GrapplingPrompt->SetVisibility(true);

	// 숨겨져 있다가 나타나는 순간 BP 이벤트 호출.
	if (!bWasVisible)
	{
		OnGrapplingPromptShown();
	}
}

void APlayerCharacter::OnCanGrapplingHookChanged(const FGameplayTag Tag, int32 NewCount)
{
	RefreshGrapplingPrompt();
}

void APlayerCharacter::StartSprint()
{
	if (ASC && SprintAbilityHandle.IsValid())
	{
		ASC->TryActivateAbility(SprintAbilityHandle);
	}
}

void APlayerCharacter::StopSprint()
{
	if (ASC && SprintAbilityHandle.IsValid())
	{
		ASC->CancelAbilityHandle(SprintAbilityHandle);
	}
}
