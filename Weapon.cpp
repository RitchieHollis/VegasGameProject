// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Vegas.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Ennemy.h"
#include "Engine/SkeletalMeshSocket.h"

AWeapon::AWeapon() {

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(GetRootComponent());

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision")); //box to detect collision with enemies
	CombatCollision->SetupAttachment(GetRootComponent());

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); //no collision by default
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); //set collision and hit particle when enemy is hit

	bWeaponParticles = false;

	WeaponState = EWeaponState::EWS_Pickup;

	Damage = 25.f;
}

void AWeapon::BeginPlay() {

	Super::BeginPlay();

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AWeapon::CombatOnOverlapEnd);
}

void AWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, 
							 AActor* OtherActor, UPrimitiveComponent* OtherComp, 
							int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) 
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if ((WeaponState == EWeaponState::EWS_Pickup) && OtherActor) {
		AVegas* Main = Cast<AVegas>(OtherActor);

		if (Main) {

			Main->SetActiveOverlappingItem(this);
		}
	}
}

void AWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {

	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	if (OtherActor) {
		AVegas* Main = Cast<AVegas>(OtherActor);

		if (Main)
			Main->SetActiveOverlappingItem(nullptr);
	}
}

void AWeapon::Equip(AVegas* Char) {

	if (Char) {

		SetInsigator(Char->GetController());

		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); //camera will ignore a weapon
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

		SkeletalMesh->SetSimulatePhysics(false);

		const USkeletalMeshSocket* LeftHandSocket = Char->GetMesh()->GetSocketByName("LeftHandSocket"); //get socket to attach the weapon to the left hand

		if (LeftHandSocket) {
			LeftHandSocket->AttachActor(this, Char->GetMesh()); //attach the weapon
			bRotate = false;
			Char->SetEquipedWeapon(this);
			Char->SetActiveOverlappingItem(nullptr);
		}
		if (OnEquipSound)
			UGameplayStatics::PlaySound2D(this, OnEquipSound); //play sound when equiped

		if (!bWeaponParticles)
			IdleParticlesComponent->Deactivate();
	}
}

void AWeapon::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
								   bool bFromSweep, const FHitResult& SweepResult) //when weapon touches enemy
{
	if (OtherActor) {

		AEnnemy* Enemy = Cast<AEnnemy>(OtherActor);
		if (Enemy) {

			if (Enemy->HitParticle) {

				const USkeletalMeshSocket* WeaponSocket = SkeletalMesh->GetSocketByName("weaponsocket");

				if (WeaponSocket) {
					FVector SocketLocation = WeaponSocket->GetSocketLocation(SkeletalMesh);
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Enemy->HitParticle, GetActorLocation(), FRotator(0.f), false); //particle system
				}
			}
			if (Enemy->HitSound) {
				UGameplayStatics::PlaySound2D(this, Enemy->HitSound);
			}
			if (DamageTypeClass) {
				UGameplayStatics::ApplyDamage(Enemy, Damage, WeaponInstigator, this, DamageTypeClass);
			}
		}
	}
}

void AWeapon::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex){


}

void AWeapon::ActivateCollision(){

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AWeapon::DeactivateCollision(){

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}