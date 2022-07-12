// Fill out your copyright notice in the Description page of Project Settings.


#include "Ennemy.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Vegas.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshcomponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/MovementComponent.h"
#include "TimerManager.h"
#include "MainPlayerController.h"

// Sets default values
AEnnemy::AEnnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Agrosphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(1100.f);
	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore); //agrosphere ignores items

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Combatsphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(75.f);

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetMesh(), FName("EnemySocket"));  //attack to socket of the arm

	bOverlapingCombatSphere = false;

	Health = 75.f;
	MaxHealth = 75.f;
	Damage = 15.f;
	AttackMinTime = 0.5f;
	AttackMaxTime = 2.5f;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	DisappearCorpseDelay = 8.f;

	bHasValidTarget = false;

}

// Called when the game starts or when spawned
void AEnnemy::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AAIController>(GetController());

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnnemy::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnnemy::CombatSphereOnOverlapEnd);

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnnemy::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnnemy::CombatOnOverlapEnd);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); //no collision by default
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); //set collision and hit particle when enemy is hit

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	
}

// Called every frame
void AEnnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, 
							  const FHitResult& SweepResult)
{
	if (OtherActor && Alive()) {

		AVegas* Vegas = Cast<AVegas>(OtherActor);
		if (Vegas) 
			MoveToTarget(Vegas);
	}
}

void AEnnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex){

	if (OtherActor) {

		AVegas* Vegas = Cast<AVegas>(OtherActor);

		if (Vegas) {

			bHasValidTarget = false;

			if (Vegas->CombatTarget == this) {

				Vegas->SetCombatTarget(nullptr);
			}

			Vegas->SetHasCombatTarget(false);

			Vegas->UpdateCombatTarget();

			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
			if (AIController)
				AIController->StopMovement();
		}
	}
}

void AEnnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, 
							    const FHitResult& SweepResult)
{
	if (OtherActor && Alive()) {

		AVegas* Vegas = Cast<AVegas>(OtherActor);

		if (Vegas) {

			bHasValidTarget = true;

			Vegas->SetCombatTarget(this);
			Vegas->SetHasCombatTarget(true);
			Vegas->UpdateCombatTarget();

			CombatTarget = Vegas;
			bOverlapingCombatSphere = true;
			float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnnemy::Attack, AttackTime); //wait specify amount of time to do Attack() function
		}
	}
}

void AEnnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex){

	if (OtherActor && OtherComp) {

		AVegas* Vegas = Cast<AVegas>(OtherActor);

		if (Vegas) {

			bOverlapingCombatSphere = false;
			AttackEnd();
			//EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;
			MoveToTarget(Vegas);
			CombatTarget = nullptr;		
			
			if (Vegas->CombatTarget == this) {

				Vegas->SetCombatTarget(nullptr);
				Vegas->bHasCombatTarget = false;
				Vegas->UpdateCombatTarget();
			}
			if (Vegas->MainPlayerController) {

				USkeletalMeshComponent* MainMesh = Cast<USkeletalMeshComponent>(OtherComp);
				if (MainMesh) Vegas->MainPlayerController->RemoveEnemyHealthBar();
			}
		
			GetWorldTimerManager().ClearTimer(AttackTimer); //clear the timer
		}
	}
}

void AEnnemy::MoveToTarget(AVegas* Target) {

	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

	if (AIController) {

		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target); //request a movement to the player
		MoveRequest.SetAcceptanceRadius(10.0f);

		FNavPathSharedPtr NavPath;

		AIController->MoveTo(MoveRequest, &NavPath);



		/*
		auto PathPoints = NavPath->GetPathPoints(); //TArray<FNavPathPoint>

		for (auto Point : PathPoints) {

			FVector Location = Point.Location; //collect every pathpoint
			UKismetSystemLibrary::DrawDebugSphere(this, Location, 50.f, 8, FLinearColor::Red, 10.f, 1.5f); //show every path point
		}
		*/
	}
}

void AEnnemy::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult) //when enemy touches the player
{
	if (OtherActor) {

		AVegas* Main = Cast<AVegas>(OtherActor);
		if (Main) {

			if (Main->HitParticle) {

				const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName("TipSocket");

				if (TipSocket) {
					FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Main->HitParticle, GetActorLocation(), FRotator(0.f), false); //particle system
				}
			}
			if (Main->HitSound) {
				UGameplayStatics::PlaySound2D(this, Main->HitSound);
			}
			if (DamageTypeClass) {

				UGameplayStatics::ApplyDamage(Main, -Damage, AIController, this, DamageTypeClass);
			}
		}
	}
}

void AEnnemy::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {


}

void AEnnemy::ActivateCollision() {

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	if (SwingSound) {
		UGameplayStatics::PlaySound2D(this, SwingSound);
	}
}

void AEnnemy::DeactivateCollision() {

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnnemy::Attack() {

	if (Alive() && bHasValidTarget) {

		if (AIController) {

			AIController->StopMovement();
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
		}
		if (!bAttacking) {
		
			bAttacking = true;
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance) {

				AnimInstance->Montage_Play(CombatMontage, 1.25f); //rsgsgqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq
				AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
				ActivateCollision();
			}
			bAttacking = false;
		}
		AttackEnd();
	}
}

void AEnnemy::AttackEnd() { 
	
	bAttacking = false; 
	DeactivateCollision();
	if (bOverlapingCombatSphere) {

		float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnnemy::Attack, AttackTime); //wait specify amount of time to do Attack() function
		//Attack();
	}
		
}

void AEnnemy::Die(AActor* Causer) {

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance) {

		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"));
	}
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bAttacking = false;

	AVegas* Main = Cast<AVegas>(Causer);

	if (Main) {

		Main->UpdateCombatTarget();
	}
}

float AEnnemy::TakeDamage(float DamageAmount,
	struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator,
	AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0.f) {

		Health -= DamageAmount;
		Die(DamageCauser);
	}
	else
		Health -= DamageAmount;

	return DamageAmount;
}

void AEnnemy::DeathEnd() {

	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;

	GetWorldTimerManager().SetTimer(CorpseTimer, this, &AEnnemy::Disappear, DisappearCorpseDelay);
}

bool AEnnemy::Alive() {

	return (GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead);
}

void AEnnemy::Disappear() {

	Destroy();
}