// Fill out your copyright notice in the Description page of Project Settings.


#include "Ridd_CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "ECustomMovementMode.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Math/Vector.h"
#include "Kismet/KismetMathLibrary.h"

FNetworkPredictionData_Client* URidd_CharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		// Return our custom client prediction class instead
		URidd_CharacterMovementComponent* MutableThis = const_cast<URidd_CharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Ridd(*this);
	}

	return ClientPredictionData;
}

bool URidd_CharacterMovementComponent::SetTarget(AActor* targetActor) {
	// If we already have a target set, return false
	if (!targetSet) {
		target = targetActor;
		targetSet = true;
		// Set the direction to the target
		ziplineDirection = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), target->GetActorLocation());
		ZiplineActive = true;
		return true;
	}
	return false;
}

bool URidd_CharacterMovementComponent::BeginZipline()
{
	// Check to make sure we have a zipline target
	if (targetSet)
	{
		// Set the movement mode to Ziplining
		SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_ZipLining);
		return true;
	}

	return false;
}

void URidd_CharacterMovementComponent::EndZipline()
{
	// check that we're ziplining. Don't change the movement mode if we aren't
	if (ZiplineActive) {
		// Set the mode to falling
		SetMovementMode(EMovementMode::MOVE_Falling);
		// TODO: Properly dispose of target
		targetSet = false;
		ZiplineActive = false;
	}
}

bool URidd_CharacterMovementComponent::NearTarget() {
	// We'll want to use server position, so verify we're on the server and ziplining
	if (GetPawnOwner()->IsLocallyControlled() == true && ZiplineActive)
	{
		FVector targetPos = target->GetActorLocation();
		FVector myPos = GetActorLocation();
		// Check that we are close enough to the actor based on the size of the distance between them
		FVector betweenPositions = myPos - targetPos;
		float distance = betweenPositions.Size();
		if (distance <= TargetDistance) {
			return true;
		}
	}
	return false;
}

void URidd_CharacterMovementComponent::OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	// If we collide with the target, stop ziplining
	// First, make sure we are ziplining
	if (ZiplineActive) {

		// Make sure what we hit was the target
		if (OtherActor == target) {
			EndZipline();
		}
	}
}


#pragma region Overrides

void URidd_CharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// We don't want simulated proxies detecting their own collision
	if (GetPawnOwner()->GetLocalRole() > ROLE_SimulatedProxy)
	{
		// Bind to the OnActorHot component so we're notified when the owning actor hits something (in my case, the target)
		GetPawnOwner()->OnActorHit.AddDynamic(this, &URidd_CharacterMovementComponent::OnActorHit);
	}
}

void URidd_CharacterMovementComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (GetPawnOwner() != nullptr && GetPawnOwner()->GetLocalRole() > ROLE_SimulatedProxy)
	{
		// Unbind from all events
		GetPawnOwner()->OnActorHit.RemoveDynamic(this, &URidd_CharacterMovementComponent::OnActorHit);
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void URidd_CharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// Peform local only checks
	if (GetPawnOwner()->IsLocallyControlled())
	{

	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void URidd_CharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	/*  There are 4 custom move flags for us to use. Below is what each is currently being used for:
		FLAG_Custom_0		= 0x10, // Ziplining
		FLAG_Custom_1		= 0x20, // Unused
		FLAG_Custom_2		= 0x40, // Unused
		FLAG_Custom_3		= 0x80, // Unused
	*/

	// Read the values from the compressed flags
	ZiplineActive = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void URidd_CharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	if (MovementMode == MOVE_Custom)
	{
		switch (CustomMovementMode)
		{
		// Start ziplining
		case ECustomMovementMode::CMOVE_ZipLining:
		{
			// Disable normal movement
		}
		break;
		}
	}

	if (PreviousMovementMode == MOVE_Custom)
	{
		switch (PreviousCustomMode)
		{
			// Stop ziplining
		case ECustomMovementMode::CMOVE_ZipLining:
		{
			// Enable normal movement
		}
		break;
		}
	}

	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

void URidd_CharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	// Phys* functions should only run for characters with ROLE_Authority or ROLE_AutonomousProxy. However, Unreal calls PhysCustom in
	// two seperate locations, one of which doesn't check the role, so we must check it here to prevent this code from running on simulated proxies.
	if (GetOwner()->GetLocalRole() == ROLE_SimulatedProxy)
		return;

	switch (CustomMovementMode)
	{
	case ECustomMovementMode::CMOVE_ZipLining:
	{
		PhysZipLining(deltaTime, Iterations);
		break;
	}
	}

	// Not sure if this is needed
	Super::PhysCustom(deltaTime, Iterations);
}

void URidd_CharacterMovementComponent::PhysZipLining(float deltaTime, int32 Iterations)
{
	// IMPORTANT NOTE: This function (and all other Phys* functions) will be called on characters with ROLE_Authority and ROLE_AutonomousProxy
	// but not ROLE_SimulatedProxy. All movement should be performed in this function so that is runs locally and on the server. UE4 will handle
	// replicating the final position, velocity, etc.. to the other simulated proxies.

	// Make sure we're in zipline mode
	if (ZiplineActive == false)
	{
		EndZipline();
		return;
	}

	// Set the owning player's new velocity
	// TODO: Need to figure these physics out
	FVector newVelocity = FVector();
	newVelocity.X *= ZipSpeed;
	newVelocity.Y *= ZipSpeed;
	newVelocity.Z *= ZipSpeed;
	Velocity = newVelocity;

	const FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);
}

float URidd_CharacterMovementComponent::GetMaxSpeed() const
{
	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
	{
		return RunSpeed;
	}
	case MOVE_Falling:
		return RunSpeed;
	case MOVE_Swimming:
		return MaxSwimSpeed;
	case MOVE_Flying:
		return MaxFlySpeed;
	case MOVE_Custom:
		return MaxCustomMovementSpeed;
	case MOVE_None:
	default:
		return 0.f;
	}
}

float URidd_CharacterMovementComponent::GetMaxAcceleration() const
{
	if (IsMovingOnGround())
	{
		return RunAcceleration;
	}

	return Super::GetMaxAcceleration();
}

#pragma endregion Overrides

void FSavedMove_Ridd::Clear()
{
	Super::Clear();

	// Clear all values
	SavedZiplineActive = 0;
}

uint8 FSavedMove_Ridd::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	/* There are 4 custom move flags for us to use. Below is what each is currently being used for:
	FLAG_Custom_0		= 0x10, // Zipline Active
	FLAG_Custom_1		= 0x20, // Unused
	FLAG_Custom_2		= 0x40, // Unused
	FLAG_Custom_3		= 0x80, // Unused
	*/

	// Write to the compressed flags 
	if (SavedZiplineActive)
		Result |= FLAG_Custom_0;

	return Result;
}

bool FSavedMove_Ridd::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* Character, float MaxDelta) const
{
	const FSavedMove_Ridd* NewMove = static_cast<const FSavedMove_Ridd*>(NewMovePtr.Get());

	// As an optimization, check if the engine can combine saved moves.
	if (SavedZiplineActive != NewMove->SavedZiplineActive)
	{
		return false;
	}

	return Super::CanCombineWith(NewMovePtr, Character, MaxDelta);
}

void FSavedMove_Ridd::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	URidd_CharacterMovementComponent* charMov = static_cast<URidd_CharacterMovementComponent*>(Character->GetCharacterMovement());
	if (charMov)
	{
		// Copy values into the saved move
		SavedZiplineActive = charMov->ZiplineActive;
	}
}

void FSavedMove_Ridd::PrepMoveFor(class ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	URidd_CharacterMovementComponent* charMov = Cast<URidd_CharacterMovementComponent>(Character->GetCharacterMovement());
	if (charMov)
	{
		// Copt values out of the saved move
		charMov->ZiplineActive = SavedZiplineActive;
	}
}

FNetworkPredictionData_Client_Ridd::FNetworkPredictionData_Client_Ridd(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{

}

FSavedMovePtr FNetworkPredictionData_Client_Ridd::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Ridd());
}



