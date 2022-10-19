// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Ridd_CharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKMOVEMENTTEST_API URidd_CharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
private:
	AActor* target;
	bool targetSet = false;

#pragma region Defaults
	// The ground speed when running
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ridd Character Movement|Grounded", Meta = (AllowPrivateAccess = "true"))
	float RunSpeed = 300.0f;
	
	// The acceleration when running
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ridd Character Movement|Grounded", Meta = (AllowPrivateAccess = "true"))
	float RunAcceleration = 2000.0f;
	
	// Speed while Ziplining
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ridd Character Movement|Ziplining", Meta = (AllowPrivateAccess = "true"))
	float ZipSpeed = 400.0f;

	// Acceleration while ziplining
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ridd Character Movement|Ziplining", Meta = (AllowPrivateAccess = "true"))
	float ZipAcceleration = 2500.0f;

	// Distance from target that we consider "reaching" it
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ridd Character Movement", Meta = (AllowPrivateAccess = "true"))
	float TargetDistance = 20.0f;



#pragma endregion Defaults


#pragma region Zipline

	// Set Target

	UFUNCTION(BlueprintCallable, Category = "Custom Character Movement")
	bool SetTarget(AActor* targetActor);

	// Begin Zipping
	UFUNCTION(BlueprintCallable, Category = "Custom Character Movement")
	bool BeginZipline();

	// End Zipping
	UFUNCTION(BlueprintCallable, Category = "Custom Character Movement")
	void EndZipline();

	// Function to check if we're close enough to the target to stop
	UFUNCTION(BlueprintCallable, Category = "Custom Character Movement")
	bool NearTarget();

private:
	UFUNCTION()
		void OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);


#pragma endregion Zipline

#pragma region Overrides
protected:
	virtual void BeginPlay() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	void PhysZipLining(float deltaTime, int32 Iterations);
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
#pragma endregion

#pragma region CompressedFlags
	uint8 ZiplineActive : 1;
#pragma endregion CompressedFlags

#pragma region Private Variables

#pragma endregion
};


class FSavedMove_Ridd : public FSavedMove_Character 
{
public:
	typedef FSavedMove_Character Super;

	// Resets all saved variables.
	virtual void Clear() override;
	// Store input commands in the compressed flags.
	virtual uint8 GetCompressedFlags() const override;
	// This is used to check whether or not two moves can be combined into one.
	// Basically you just check to make sure that the saved variables are the same.
	virtual bool CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* Character, float MaxDelta) const override;
	// Sets up the move before sending it to the server. 
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
	// Sets variables on character movement component before making a predictive correction.
	virtual void PrepMoveFor(class ACharacter* Character) override;

private:
	uint8 SavedZiplineActive : 1;
};

class FNetworkPredictionData_Client_Ridd : public FNetworkPredictionData_Client_Character
{
public:
	typedef FNetworkPredictionData_Client_Character Super;

	// Constructor
	FNetworkPredictionData_Client_Ridd(const UCharacterMovementComponent& ClientMovement);

	//brief Allocates a new copy of our custom saved move
	virtual FSavedMovePtr AllocateNewMove() override;
};
