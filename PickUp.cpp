// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp.h"
#include "Vegas.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

APickUp::APickUp() {

}

void APickUp::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	//UE_LOG(LogTemp, Warning, TEXT("PickUp::OnOverlapBegin succedded")); //test for collision

	if (OtherActor) { //check actor who collided

		AVegas* Main = Cast<AVegas>(OtherActor); //otheractor is vegas

		if (Main) { 

			OnPickupBP(Main);

			Main->PickupLocations.Add(GetActorLocation()); //everytime we pick up coin, his location is storred in PickupLocations

			if (OverlapParticles != nullptr)  //particle when collected
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticles, GetActorLocation(), FRotator(0.f), true);

			if (OverlapSound)  //sound when collected
				UGameplayStatics::PlaySound2D(this, OverlapSound);

			Destroy();
		}
	}
}

void APickUp::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {

	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	//UE_LOG(LogTemp, Warning, TEXT("Pickup::OnOverlapEnd succedded")); //test for collision
}