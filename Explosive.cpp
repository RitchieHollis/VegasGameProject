// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"
#include "Vegas.h"
#include "Ennemy.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

AExplosive::AExplosive() {

	Damage = 20.f;

}

void AExplosive::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
								int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) 
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	//UE_LOG(LogTemp, Warning, TEXT("Explosive::OnOverlapBegin succedded")); //test for collision

	if (OtherActor) { //check actor who collided
		
		AVegas* Main = Cast<AVegas>(OtherActor); //otheractor is vegas
		AEnnemy* Enemy = Cast<AEnnemy>(OtherActor);

		if (Main || Enemy) {

			if (OverlapParticles != nullptr)  //particle when collected
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticles, GetActorLocation(), FRotator(0.f), true);

			if (OverlapSound)  //sound when collected
				UGameplayStatics::PlaySound2D(this, OverlapSound);

			UGameplayStatics::ApplyDamage(OtherActor, -Damage, nullptr, this, DamageTypeClass); //to check &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

			Destroy();
		}	
	}
}

void AExplosive::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {

	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	//UE_LOG(LogTemp, Warning, TEXT("Explosive::OnOverlapEnd succedded")); //test for collision
}