#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/GameViewportSubsystem.h"
#include "Characters/EnemyCharacter.h"
#include "Characters/PlayerCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/WidgetComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayEffect.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "GAS/Attributes/BaseAttributeSet.h"
#include "GAS/Attributes/PlayerAttributeSet.h"
#include "GAS/Tags/GASGameplayTags.h"
#include "UI/Widgets/PlayerVitalsWidget.h"
#include "UI/Widgets/VitalsWidget.h"
#include "UObject/StrongObjectPtr.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVitalsAttackTest, "GASProject.UI.Vitals.AttackUpdatesEnemyHealth",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVitalsAttackTest::RunTest(const FString& Parameters)
{
	UClass* PlayerClass = LoadClass<APlayerCharacter>(nullptr,
		TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.BP_ThirdPersonCharacter_C"));
	UClass* EnemyClass = LoadClass<AEnemyCharacter>(nullptr,
		TEXT("/Game/ThirdPerson/Blueprints/BP_EnemyCharacter.BP_EnemyCharacter_C"));
	UClass* AttackClass = LoadClass<UGameplayAbility>(nullptr,
		TEXT("/Game/ThirdPerson/GameAbility/BPGA_AttackA.BPGA_AttackA_C"));
	if (!TestNotNull(TEXT("Player BP"), PlayerClass) || !TestNotNull(TEXT("Enemy BP"), EnemyClass)
		|| !TestNotNull(TEXT("Attack ability BP"), AttackClass))
	{
		return false;
	}

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Transient gameplay world"), World))
	{
		return false;
	}
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	ON_SCOPE_EXIT
	{
		World->DestroyWorld(false);
		GEngine->DestroyWorldContext(World);
	};
	World->InitializeActorsForPlay(FURL());

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APlayerCharacter* Player = World->SpawnActor<APlayerCharacter>(PlayerClass,
		FVector(0.0f, 0.0f, 100.0f), FRotator::ZeroRotator, SpawnParameters);
	AEnemyCharacter* Enemy = World->SpawnActor<AEnemyCharacter>(EnemyClass,
		FVector(100.0f, 0.0f, 100.0f), FRotator::ZeroRotator, SpawnParameters);
	if (!TestNotNull(TEXT("Player actor"), Player) || !TestNotNull(TEXT("Enemy actor"), Enemy))
	{
		return false;
	}
	Player->DispatchBeginPlay();
	Enemy->DispatchBeginPlay();

	UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent();
	UAbilitySystemComponent* EnemyASC = Enemy->GetAbilitySystemComponent();
	const float InitialHealth = EnemyASC->GetNumericAttribute(UBaseAttributeSet::GetHealthAttribute());
	const float MaxHealth = EnemyASC->GetNumericAttribute(UBaseAttributeSet::GetMaxHealthAttribute());
	TestTrue(TEXT("Enemy health initialized"), InitialHealth > 0.0f && MaxHealth >= InitialHealth);

	UWidgetComponent* Component = Enemy->FindComponentByClass<UWidgetComponent>();
	UVitalsWidget* Widget = Component ? Cast<UVitalsWidget>(Component->GetUserWidgetObject()) : nullptr;
	if (!TestNotNull(TEXT("Enemy head widget"), Widget))
	{
		return false;
	}
	TestEqual(TEXT("Head widget targets the enemy ASC"), Widget->GetTargetASC(), EnemyASC);
	UProgressBar* HealthBar = Cast<UProgressBar>(Widget->GetWidgetFromName(TEXT("HealthBar")));
	UProgressBar* ManaBar = Cast<UProgressBar>(Widget->GetWidgetFromName(TEXT("ManaBar")));
	if (!TestNotNull(TEXT("HealthBar binding"), HealthBar) || !TestNotNull(TEXT("ManaBar binding"), ManaBar))
	{
		return false;
	}
	TestTrue(TEXT("Head health bar has no screen-sized translation"),
		HealthBar->GetRenderTransform().Translation.IsNearlyZero());
	TestTrue(TEXT("Head health bar has normal scale"),
		HealthBar->GetRenderTransform().Scale.Equals(FVector2D(1.0f, 1.0f)));
	const float InitialManaPercent = ManaBar->GetPercent();

	// 실제 공격 BP의 이벤트 대기, 충돌 검사, 데미지 이펙트 경로를 실행한다.
	if (!TestTrue(TEXT("Activate first combo attack"), PlayerASC->TryActivateAbilityByClass(AttackClass)))
	{
		return false;
	}
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Player, HitTraceTag, FGameplayEventData());
	const float HealthAfterHit = EnemyASC->GetNumericAttribute(UBaseAttributeSet::GetHealthAttribute());
	AddInfo(FString::Printf(TEXT("Enemy health: %.1f -> %.1f; health bar: %.3f"),
		InitialHealth, HealthAfterHit, HealthBar->GetPercent()));
	TestTrue(TEXT("Attack reduces enemy health by 10"),
		FMath::IsNearlyEqual(HealthAfterHit, InitialHealth - 10.0f));
	TestTrue(TEXT("Health bar follows the changed enemy attribute"),
		FMath::IsNearlyEqual(HealthBar->GetPercent(), HealthAfterHit / MaxHealth));
	TestTrue(TEXT("Damage leaves mana unchanged"),
		FMath::IsNearlyEqual(ManaBar->GetPercent(), InitialManaPercent));
	PlayerASC->CancelAllAbilities();
	return true;
}
namespace
{
class FPlayerSprintVitalsCommand : public IAutomationLatentCommand
{
public:
	FPlayerSprintVitalsCommand(FAutomationTestBase* InTest, UWorld* InWorld, APlayerCharacter* InPlayer,
		UPlayerVitalsWidget* InWidget, UEnhancedInputComponent* InInput, const UInputAction* InSprintAction,
		UClass* InDrainClass, float InWalkSpeed, float InStamina, float InPeriod)
		: Test(InTest), World(InWorld), Player(InPlayer), Widget(InWidget), Input(InInput),
		SprintAction(InSprintAction), DrainClass(InDrainClass), WalkSpeed(InWalkSpeed),
		InitialStamina(InStamina), Period(InPeriod)
	{
	}

	virtual ~FPlayerSprintVitalsCommand() override
	{
		UGameViewportSubsystem::Get()->RemoveWidget(Widget.Get());
		World->DestroyWorld(false);
		GEngine->DestroyWorldContext(World);
	}

	virtual bool Update() override
	{
		if (!bTimersPrimed)
		{
			World->Tick(LEVELTICK_All, 0.0f);
			bTimersPrimed = true;
			return false;
		}

		// 다음 엔진 프레임에서 실제 GE 주기 타이머를 진행한다.
		World->Tick(LEVELTICK_All, Period + 0.1f);
		UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent();
		const float Stamina = ASC->GetNumericAttribute(UPlayerAttributeSet::GetStaminaAttribute());
		const float MaxStamina = ASC->GetNumericAttribute(UPlayerAttributeSet::GetMaxStaminaAttribute());
		UProgressBar* StaminaBar = CastChecked<UProgressBar>(Widget->GetWidgetFromName(TEXT("StaminaBar")));
		Test->AddInfo(FString::Printf(TEXT("Player sprint speed: %.1f -> %.1f; stamina: %.1f -> %.1f; stamina bar: %.3f"),
			WalkSpeed, Player->GetCharacterMovement()->MaxWalkSpeed, InitialStamina, Stamina, StaminaBar->GetPercent()));
		Test->TestTrue(TEXT("Periodic sprint GE consumes stamina"), Stamina < InitialStamina);
		Test->TestTrue(TEXT("Player stamina bar follows the attribute"),
			FMath::IsNearlyEqual(StaminaBar->GetPercent(), Stamina / MaxStamina));

		ExecuteSprintEvent(ETriggerEvent::Completed);
		Test->TestEqual(TEXT("Release restores walk speed"), Player->GetCharacterMovement()->MaxWalkSpeed, WalkSpeed);
		Test->TestFalse(TEXT("Release removes sprint tag"), ASC->HasMatchingGameplayTag(SprintingTag));
		Test->TestFalse(TEXT("Release removes regeneration block"), ASC->HasMatchingGameplayTag(StaminaRegenBlockedTag));
		FGameplayEffectQuery Query;
		Query.EffectDefinition = DrainClass;
		Test->TestEqual(TEXT("Release removes stamina drain GE"), ASC->GetActiveEffects(Query).Num(), 0);

		ExecuteSprintEvent(ETriggerEvent::Started);
		Test->TestTrue(TEXT("Sprint can restart"), ASC->HasMatchingGameplayTag(SprintingTag));
		ExecuteSprintEvent(ETriggerEvent::Canceled);
		Test->TestFalse(TEXT("Canceled input ends sprint"), ASC->HasMatchingGameplayTag(SprintingTag));
		Test->TestEqual(TEXT("Canceled input restores speed"), Player->GetCharacterMovement()->MaxWalkSpeed, WalkSpeed);

		ExecuteSprintEvent(ETriggerEvent::Started);
		ASC->SetNumericAttributeBase(UPlayerAttributeSet::GetStaminaAttribute(), 0.0f);
		Test->TestFalse(TEXT("Empty stamina automatically ends sprint"), ASC->HasMatchingGameplayTag(SprintingTag));
		Test->TestEqual(TEXT("Empty stamina restores speed"), Player->GetCharacterMovement()->MaxWalkSpeed, WalkSpeed);
		Test->TestEqual(TEXT("Empty stamina updates the bar"), StaminaBar->GetPercent(), 0.0f);
		ExecuteSprintEvent(ETriggerEvent::Started);
		Test->TestFalse(TEXT("Empty stamina prevents sprint restart"), ASC->HasMatchingGameplayTag(SprintingTag));
		return true;
	}

private:
	void ExecuteSprintEvent(ETriggerEvent Event)
	{
		for (const TUniquePtr<FEnhancedInputActionEventBinding>& Binding : Input->GetActionEventBindings())
		{
			if (Binding->GetAction() == SprintAction && Binding->GetTriggerEvent() == Event)
			{
				Binding->Execute(FInputActionInstance(SprintAction));
				return;
			}
		}
		Test->AddError(TEXT("Sprint input event binding is missing"));
	}

	FAutomationTestBase* Test;
	UWorld* World;
	APlayerCharacter* Player;
	TStrongObjectPtr<UPlayerVitalsWidget> Widget;
	TStrongObjectPtr<UEnhancedInputComponent> Input;
	const UInputAction* SprintAction;
	UClass* DrainClass;
	float WalkSpeed;
	float InitialStamina;
	float Period;
	bool bTimersPrimed = false;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlayerSprintVitalsTest, "GASProject.UI.Vitals.PlayerSprintAndViewport",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerSprintVitalsTest::RunTest(const FString& Parameters)
{
	UClass* PlayerClass = LoadClass<APlayerCharacter>(nullptr,
		TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.BP_ThirdPersonCharacter_C"));
	UClass* WidgetClass = LoadClass<UPlayerVitalsWidget>(nullptr,
		TEXT("/Game/UI/WBP_PlayerVitals.WBP_PlayerVitals_C"));
	UClass* DrainClass = LoadClass<UGameplayEffect>(nullptr,
		TEXT("/Game/ThirdPerson/GameEffects/Effects/MoveEffects/BPGE_SprintStamina.BPGE_SprintStamina_C"));
	UInputAction* SprintAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_Sprint.IA_Sprint"));
	UInputMappingContext* Mapping = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Input/IMC_Default.IMC_Default"));
	if (!TestNotNull(TEXT("Player BP"), PlayerClass) || !TestNotNull(TEXT("Player widget BP"), WidgetClass)
		|| !TestNotNull(TEXT("Sprint drain BP"), DrainClass) || !TestNotNull(TEXT("Sprint action"), SprintAction)
		|| !TestNotNull(TEXT("Input mapping context"), Mapping))
	{
		return false;
	}
	bool bShiftMapped = false;
	for (const FEnhancedActionKeyMapping& MappingEntry : Mapping->GetMappingsForProfile(FString()))
	{
		bShiftMapped |= MappingEntry.Action == SprintAction && MappingEntry.Key == EKeys::LeftShift;
	}
	TestTrue(TEXT("Left Shift maps to IA_Sprint"), bShiftMapped);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	bool bHandedToLatentCommand = false;
	ON_SCOPE_EXIT
	{
		if (!bHandedToLatentCommand)
		{
			World->DestroyWorld(false);
			GEngine->DestroyWorldContext(World);
		}
	};
	World->InitializeActorsForPlay(FURL());
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APlayerCharacter* Player = World->SpawnActor<APlayerCharacter>(PlayerClass,
		FVector(0.0f, 0.0f, 100.0f), FRotator::ZeroRotator, SpawnParameters);
	Player->DispatchBeginPlay();
	UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent();
	UPlayerVitalsWidget* Widget = NewObject<UPlayerVitalsWidget>(World, WidgetClass);
	if (!TestTrue(TEXT("Player widget initializes"), Widget->Initialize()))
	{
		return false;
	}
	Widget->SetTargetASC(ASC);
	UProgressBar* HealthBar = Cast<UProgressBar>(Widget->GetWidgetFromName(TEXT("HealthBar")));
	UProgressBar* StaminaBar = Cast<UProgressBar>(Widget->GetWidgetFromName(TEXT("StaminaBar")));
	if (!TestNotNull(TEXT("Player HealthBar binding"), HealthBar)
		|| !TestNotNull(TEXT("Player StaminaBar binding"), StaminaBar))
	{
		return false;
	}
	TestEqual(TEXT("Player widget targets player ASC"), Widget->GetTargetASC(), ASC);
	TestEqual(TEXT("Player health bar initially full"), HealthBar->GetPercent(), 1.0f);
	TestEqual(TEXT("Player stamina bar initially full"), StaminaBar->GetPercent(), 1.0f);
	Widget->SetPlayerViewportLayout(FVector2D(300.0f, 72.0f), FVector2D(24.0f, -24.0f));
	const FGameViewportWidgetSlot Slot = UGameViewportSubsystem::Get()->GetWidgetSlot(Widget);
	TestTrue(TEXT("Player HUD keeps bottom-left anchors"),
		Slot.Anchors.Minimum.Equals(FVector2D(0.0f, 1.0f)) && Slot.Anchors.Maximum.Equals(FVector2D(0.0f, 1.0f)));
	TestTrue(TEXT("Player HUD keeps bottom-left alignment"), Slot.Alignment.Equals(FVector2D(0.0f, 1.0f)));
	TestTrue(TEXT("Player HUD keeps size and offsets"),
		Slot.Offsets == FMargin(24.0f, -24.0f, 300.0f, 72.0f));

	UEnhancedInputComponent* Input = NewObject<UEnhancedInputComponent>(Player);
	Player->SetupPlayerInputComponent(Input);
	const float WalkSpeed = Player->GetCharacterMovement()->MaxWalkSpeed;
	const float InitialStamina = ASC->GetNumericAttribute(UPlayerAttributeSet::GetStaminaAttribute());
	for (const TUniquePtr<FEnhancedInputActionEventBinding>& Binding : Input->GetActionEventBindings())
	{
		if (Binding->GetAction() == SprintAction && Binding->GetTriggerEvent() == ETriggerEvent::Started)
		{
			Binding->Execute(FInputActionInstance(SprintAction));
			break;
		}
	}
	if (!TestTrue(TEXT("Sprint input activates ability"), ASC->HasMatchingGameplayTag(SprintingTag)))
	{
		UGameViewportSubsystem::Get()->RemoveWidget(Widget);
		return false;
	}
	TestTrue(TEXT("Sprint increases movement speed"), Player->GetCharacterMovement()->MaxWalkSpeed > WalkSpeed);
	TestTrue(TEXT("Sprint blocks regeneration"), ASC->HasMatchingGameplayTag(StaminaRegenBlockedTag));
	const float Period = DrainClass->GetDefaultObject<UGameplayEffect>()->Period.GetValueAtLevel(1.0f);
	TestTrue(TEXT("Sprint drain GE has a positive period"), Period > 0.0f);
	ADD_LATENT_AUTOMATION_COMMAND(FPlayerSprintVitalsCommand(this, World, Player, Widget, Input,
		SprintAction, DrainClass, WalkSpeed, InitialStamina, Period));
	bHandedToLatentCommand = true;
	return true;
}

#endif
