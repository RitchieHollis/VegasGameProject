// Fill out your copyright notice in the Description page of Project Settings.


#include "Vegas.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Sound/SoundCue.h"
#include "Ennemy.h"
#include "MainPlayerController.h"
#include "FirstSaveGame.h"
#include "ItemStorage.h"

// Sets default values
AVegas::AVegas()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create camera boom (pulls towards the player when collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f; //camera follows at this distante
	CameraBoom->bUsePawnControlRotation = true; //rotate arm based on controller

	//Set Size for Collision Capsule
	GetCapsuleComponent()->SetCapsuleSize(56.679256f, 109.031357f);

	//Create follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); //attached to the end of spring arm
	FollowCamera->bUsePawnControlRotation = false; //adjust the controller orientation

	//Set our turn rates for input
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	//Character will not follow the camera
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	//Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;			 //Elvis moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.0f);  //... at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 550.f; //height of a jump
	GetCharacterMovement()->AirControl = 0.3f; //moving while falling

	//Player Stats 
	MaxHealth = 100.f;
	Health = 100.f;
	MaxStamina = 325.f;
	Stamina = 325.f;
	Coins = 0;

	RunningSpeed = 650.f;
	SprintSpeed = 950.f;

	bShiftKey = false;

	//initalize enums

	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.f;
	MinSprintStamina = 75.f;

	bFlipKeyPressed = false;
	bLMBDown = false;

	InterpSpeed = 15.f;
	bInterpToEnemy = false;

	bHasCombatTarget = false;

	bMovingFB = false;
	bMovingS = false;

}

// Called when the game starts or when spawned
void AVegas::BeginPlay()
{
	Super::BeginPlay();
	
	MainPlayerController = Cast<AMainPlayerController>(GetController());
}

// Called every frame
void AVegas::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementStatus == EMovementStatus::EMS_Dead) return;
	//------------------------------------------------------
	//stamina
	float DeltaStamina = StaminaDrainRate * DeltaTime;
	switch (StaminaStatus) {

		case EStaminaStatus::ESS_Normal:
			if (bShiftKey) {

				if (Stamina - DeltaStamina <= MinSprintStamina) {
					SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
					Stamina -= DeltaStamina;
				}
				else Stamina -= DeltaStamina;
				if(bMovingFB || bMovingS)
					SetMovementStatus(EMovementStatus::EMS_Sprinting);
				else SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			else { //shift key up

				if (Stamina + DeltaStamina >= MaxStamina)
					Stamina = MaxStamina;
				else Stamina += DeltaStamina;
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			break;

		case EStaminaStatus::ESS_BelowMinimum:
			if (bShiftKey) {
				
				if (Stamina - DeltaStamina <= 0.f) {
					SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
					Stamina = 0;
					SetMovementStatus(EMovementStatus::EMS_Normal);
				}
				else { 
					Stamina -= DeltaStamina; 
					if (bMovingFB || bMovingS)
						SetMovementStatus(EMovementStatus::EMS_Sprinting);
					else SetMovementStatus(EMovementStatus::EMS_Normal);
				}
			}
			else {

				if (Stamina + DeltaStamina >= MinSprintStamina) {
					SetStaminaStatus(EStaminaStatus::ESS_Normal);
					Stamina += DeltaStamina;
				}
				else Stamina += DeltaStamina;
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			break;

		case EStaminaStatus::ESS_Exhausted:
			if (bShiftKey)
				Stamina = 0.f;
			else {
				SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
			break;

		case EStaminaStatus::ESS_ExhaustedRecovering:
			if (Stamina + DeltaStamina >= MinSprintStamina) {
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
				Stamina += DeltaStamina;
			}
			else Stamina += DeltaStamina;
			SetMovementStatus(EMovementStatus::EMS_Normal);
			break;
	}
	//--------------------------------------------------------
	//interpelating

	if (bInterpToEnemy && CombatTarget) {

		FRotator LookAtYawn = GetLookAtRotationYawn(CombatTarget->GetActorLocation()); //rotation to look directly at enemy when attacking
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYawn, DeltaTime, InterpSpeed); //smooth transition when rotate

		SetActorRotation(InterpRotation);
	}
	//----------------------------------------------------------
	//position of enemy

	if (CombatTarget) {

		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (MainPlayerController)
			MainPlayerController->EnemyLocation = CombatTargetLocation;
	}
}

//--------------------------------------------------------------------Movement--------------------------------------------------------------------//

FRotator AVegas::GetLookAtRotationYawn(FVector Target) {

	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYawn(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYawn;
}

// Called to bind functionality to input
void AVegas::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AVegas::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AVegas::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AVegas::ShiftKeyUp);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AVegas::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AVegas::LMBUp);

	//PlayerInputComponent->BindAction("Flip", IE_Pressed, this, &AVegas::FlipDown);

	PlayerInputComponent->BindAxis("MoveForwardBack", this, &AVegas::MoveForwardBack); //movement
	PlayerInputComponent->BindAxis("MoveSides", this, &AVegas::MoveSides);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &AVegas::AddControllerPitchInput); //camera
	PlayerInputComponent->BindAxis("TurnRate", this, &AVegas::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AVegas::LookUpAtRate);

}

void AVegas::MoveForwardBack(float Value) {

	bMovingFB = false;

	if ((Controller != nullptr) && (Value != 0.0f) && (!bAttacking) && (MovementStatus != EMovementStatus::EMS_Dead)) {

		bMovingFB = true;
		//find out which way is Forward
		const FRotator Rotation = Controller->GetControlRotation(); //Direction that the controller is facing
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		//Forward Vector of the controller
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); //rotation matrix X forward-back

		AddMovementInput(Direction, Value);
	}
}

void AVegas::MoveSides(float Value) {

	bMovingS = false;

	if ((Controller != nullptr) && (Value != 0.0f) && (!bAttacking) && (MovementStatus != EMovementStatus::EMS_Dead)) {

		bMovingS = true;
		//find out which way is Forward
		const FRotator Rotation = Controller->GetControlRotation(); //Direction that the controller is facing
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		//Forward Vector of the controller
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y); //rotation matrix Y sides

		AddMovementInput(Direction, Value);
	}
}

void AVegas::TurnAtRate(float Rate) {

	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AVegas::LookUpAtRate(float Rate) {

	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AVegas::Jump() {

	if (MovementStatus != EMovementStatus::EMS_Dead) {

		ACharacter::Jump();
	}
}

void AVegas::ShiftKeyDown() { bShiftKey = true; }

void AVegas::ShiftKeyUp() { bShiftKey = false; }

void AVegas::SetMovementStatus(EMovementStatus Status) {

	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting)
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	else
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;

}

//--------------------------------------------------------------------Items--------------------------------------------------------------------//

void AVegas::ChangeCoins(float Amount) {

	if (Coins + Amount < 999.f)
		Coins += Amount;
}

void AVegas::ShowPickupLocations() {

	for (FVector Location : PickupLocations) {
		UKismetSystemLibrary::DrawDebugSphere(this, Location, 50.f, 8, FLinearColor::Green, 10.f, 0.5f); //show collected coins in debug mode
	}
}

void AVegas::SetEquipedWeapon(AWeapon* WeaponToSet) {

	if (EquipedWeapon)
		EquipedWeapon->Destroy();

	EquipedWeapon = WeaponToSet;
}

//--------------------------------------------------------------------Combat--------------------------------------------------------------------//

void AVegas::LMBDown() { 
	
	bLMBDown = true; 

	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	if (ActiveOverlappingItem) {

		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon) {
			Weapon->Equip(this);
			SetActiveOverlappingItem(nullptr);
		}
	}
	else if (EquipedWeapon && !(GetCharacterMovement()->IsFalling()))
		Attack();
	

}

void AVegas::LMBUp() { bLMBDown = false; }

void AVegas::Attack() {

	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead) {

		bAttacking = true;
		SetInterpToEnemy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && CombatMontage) {

			AnimInstance->Montage_Play(CombatMontage, 1.35f);
			AnimInstance->Montage_JumpToSection(FName("Attack1"), CombatMontage);
		}

		if (EquipedWeapon->SwingSound) {
			UGameplayStatics::PlaySound2D(this, EquipedWeapon->SwingSound);
		}
	}
}

void AVegas::AttackEnd() { 
	
	bAttacking = false;
	SetInterpToEnemy(false);
	if (bLMBDown)
		Attack();
}

void AVegas::SetInterpToEnemy(bool Interp) {

	bInterpToEnemy = Interp;
}

float AVegas::TakeDamage(float DamageAmount,
	struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator,
	AActor* DamageCauser) 
{
	if (Health + DamageAmount <= 0.f)
	{
		Health += DamageAmount;
		Die();
		if (DamageCauser) {

			AEnnemy* Enemy = Cast <AEnnemy>(DamageCauser);
			if (Enemy)
				Enemy->bHasValidTarget = false;
		}
	}
	else
		Health += DamageAmount;

	return DamageAmount;
}

void AVegas::ChangeHealth(float Amount) {

	if (Health + Amount <= 0.f)
	{
		Health += Amount;
		Die();
	}
	else if (Health + Amount >= MaxHealth) {

		Health = MaxHealth;
	}
	else
		Health += Amount;
}

void AVegas::UpdateCombatTarget() {

	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, EnemyFilter);

	if (OverlappingActors.Num() == 0) {

		if (MainPlayerController) {

			MainPlayerController->RemoveEnemyHealthBar();
		}
		return;
	}

	AEnnemy* ClosestEnemy = Cast<AEnnemy>(OverlappingActors[0]);
	
	if (ClosestEnemy) {

		FVector Location = GetActorLocation();
		float MinDistance = (ClosestEnemy->GetActorLocation() - GetActorLocation()).Size(); //distance between player and enemy

		for (auto Actor : OverlappingActors) {

			AEnnemy* Enemy = Cast<AEnnemy>(Actor);
			if (Enemy) {

				float DistanceToActor = (Enemy->GetActorLocation() - Location).Size();

				if (DistanceToActor < MinDistance) {

					MinDistance = DistanceToActor;
					ClosestEnemy = Enemy;
				}
			} //looking for the closest enemy each loop
		}

		if (MainPlayerController) {

			MainPlayerController->DisplayEnemyHealthBar();
		}

		SetCombatTarget(ClosestEnemy);
		bHasCombatTarget = true;
	}
}

void AVegas::Die() {

	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && CombatMontage) {

		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"));
	}
	SetMovementStatus(EMovementStatus::EMS_Dead);
	DeathEnd();
}

void AVegas::DeathEnd() {
	
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}

//--------------------------------------------------------------------Change-Level--------------------------------------------------------------------//

void AVegas::SwitchLevel(FName LevelName) {

	UWorld* World = GetWorld();

	if (World) {

		FString CurrentLevel = World->GetMapName();
		FName CurrentLevelName(*CurrentLevel); //switch string to name

		if (CurrentLevelName != LevelName) {

			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}

//--------------------------------------------------------------------Saving-Loading--------------------------------------------------------------------//

void AVegas::SaveGame() {

	
	//USaveGame -> UFirstSaveGame -> instance in UFirstSaveGame
	UFirstSaveGame* SaveGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass())); //== TSubclassOf<UFirstSaveGame>

	FString CurrentLevel = GetWorld()->GetMapName();
	CurrentLevel.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	FName CurrentLevelName(*CurrentLevel);
	
	SaveGameInstance->CharacterStats.Health = Health;
	if (SaveGameInstance->CharacterStats.Health)
		UE_LOG(LogTemp, Warning, TEXT("HealthStored"));

	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;     //saving values
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStats.Coins = Coins;
	SaveGameInstance->CharacterStats.MapName = CurrentLevelName;

	if(EquipedWeapon)
		SaveGameInstance->CharacterStats.WeaponName = EquipedWeapon->Name;

	SaveGameInstance->CharacterStats.Location = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
}


void AVegas::LoadGame(bool SetPosition) {

	UFirstSaveGame* LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));

	LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex)); //override loadgameinstance

	//UGameplayStatics::OpenLevel(GetWorld(), LoadGameInstance->CharacterStats.MapName);

	if (LoadGameInstance->CharacterStats.MapName != TEXT("")) {

		FName LevelName(LoadGameInstance->CharacterStats.MapName);
		SwitchLevel(LevelName);
	}

	if (LoadGameInstance->CharacterStats.Health == 80.f)
		UE_LOG(LogTemp, Warning, TEXT("HealthIsStored"));

	Health = LoadGameInstance->CharacterStats.Health; 
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;
	
	if (WeaponStorage) {

		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons) {
			
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			if (Weapons->WeaponMap.Contains(WeaponName)) {

				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]); //create new weapon and spawn it depending of blueprint
				WeaponToEquip->Equip(this);
			}
		}
	}

	if (SetPosition) {

		SetActorLocation(LoadGameInstance->CharacterStats.Location);
		SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
	}
	
}

//--------------------------------------------------------------------Test--------------------------------------------------------------------//

void AVegas::Flip(float Value) {

	bFlip = true;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && !bPressedJump) {

		AnimInstance->Montage_Play(CombatMontage, 1.35f);
		AnimInstance->Montage_JumpToSection(FName("Flip"), CombatMontage);

		const FRotator Rotation = Controller->GetControlRotation(); //Direction that the controller is facing
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		//Forward Vector of the controller
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); //rotation matrix X forward-back

		AddMovementInput(Direction, Value);
	}
}

void AVegas::FlipDown() {

	bFlipDown = true;

	Flip(1.f);
}