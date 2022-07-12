// Fill out your copyright notice in the Description page of Project Settings.


#include "VegasAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Vegas.h"

void UVegasAnimInstance::NativeInitializeAnimation() {

	if (Pawn == nullptr) {
		
		Pawn = TryGetPawnOwner(); //check what is the pawn that owns animation instance, we can store it here
		if (Pawn)
			Vegas = Cast<AVegas>(Pawn);
	}
}

void UVegasAnimInstance::UpdateAnimationProperties() {

	if (Pawn == nullptr)
		Pawn = TryGetPawnOwner();

	if (Pawn) {

		FVector Speed = Pawn->GetVelocity();
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
		MovementSpeed = LateralSpeed.Size();

		bIsInAir = Pawn->GetMovementComponent()->IsFalling();

		if(Vegas == nullptr)
			Vegas = Cast<AVegas>(Pawn);
	}
}
