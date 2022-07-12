// Fill out your copyright notice in the Description page of Project Settings.


#include "EnnemyAnimInstance.h"
#include "Ennemy.h"

void UEnnemyAnimInstance::NativeInitializeAnimation() {

	if (Pawn == nullptr) {

		Pawn = TryGetPawnOwner(); //check what is the pawn that owns animation instance, we can store it here
		if (Pawn)
			Ennemy = Cast<AEnnemy>(Pawn);
	}
}

void UEnnemyAnimInstance::UpdateAnimationProperties() {

	if (Pawn == nullptr) {
		Pawn = TryGetPawnOwner();

		if (Pawn) 
			Ennemy = Cast<AEnnemy>(Pawn);
		
	}
	if (Pawn) {

		FVector Speed = Pawn->GetVelocity();
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
		MovementSpeed = LateralSpeed.Size();
	}
}
