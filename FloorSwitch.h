// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloorSwitch.generated.h"

UCLASS()
class FIRSTGAME_API AFloorSwitch : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFloorSwitch();

	/** Overlap volume for functionality to be triggered */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = FloorSwitch)
	class UBoxComponent* TriggerBox;

	/** Switch for the character to step on */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = FloorSwitch)
	class UStaticMeshComponent* FloorSwitch;

	/** Door to move when the floor switch is stepped on*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = FloorSwitch)
	UStaticMeshComponent* Door;

	/** Initial location for the door*/
	UPROPERTY(BlueprintReadWrite, Category = FloorSwitch)
	FVector InitialDoorLocation;

	/** Initial location for the switch*/
	UPROPERTY(BlueprintReadWrite, Category = FloorSwitch)
	FVector InitialSwitchLocation;

	/** Timer to close the door*/
	FTimerHandle SwitchHandle; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FloorSwitch)
	float SwitchTime;

	bool bVegasOnSwitch;

	void CloseDoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* Comp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = FloorSwitch)
	void RaiseDoor(); 

	UFUNCTION(BlueprintImplementableEvent, Category = FloorSwitch)
	void LowerDoor(); 

	UFUNCTION(BlueprintImplementableEvent, Category = FloorSwitch)
	void RaiseFloorSwitch();

	UFUNCTION(BlueprintImplementableEvent, Category = FloorSwitch)
	void LowerFloorSwitch();

	UFUNCTION(BlueprintCallable, Category = FloorSwitch)
	void UpdateDoorLocation(float z);

	UFUNCTION(BlueprintCallable, Category = FloorSwitch)
	void UpdateFloorSwitchLocation(float z);
};
