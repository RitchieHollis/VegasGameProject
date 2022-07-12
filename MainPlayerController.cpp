// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"

void AMainPlayerController::BeginPlay() {

	Super::BeginPlay(); //1st thing to do when overrading BeginPlay()

	if (HUDOverlayAsset) 
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset); //templet constructor for widgets

	HUDOverlay->AddToViewport();
	HUDOverlay->SetVisibility(ESlateVisibility::Visible); //set HUD for visible

	if (WEnemyHealthBar) {

		EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);
		if (EnemyHealthBar) {

			EnemyHealthBar->AddToViewport();
			EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
		}
		FVector2D Alignment(0.f, 0.f);
		EnemyHealthBar->SetAlignmentInViewport(Alignment);
	}
}

void AMainPlayerController::Tick(float DeltaTime) {

	Super::Tick(DeltaTime);

	if (EnemyHealthBar) {

		FVector2D PositionInViewport;
		ProjectWorldLocationToScreen(EnemyLocation, PositionInViewport);
		PositionInViewport.Y -= 85.f;

		FVector2D SizeInViewport(250.f, 25.f);

		EnemyHealthBar->SetPositionInViewport(PositionInViewport);
		EnemyHealthBar->SetDesiredSizeInViewport(SizeInViewport);
	}
}

void AMainPlayerController::DisplayEnemyHealthBar() {

	if (EnemyHealthBar) {

		bEnemyHeathBarVisible = true;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMainPlayerController::RemoveEnemyHealthBar() {

	if (EnemyHealthBar) {

		bEnemyHeathBarVisible = false;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
	}
}