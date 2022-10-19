// Fill out your copyright notice in the Description page of Project Settings.


#include "Ridd_Character.h"
#include "Ridd_CharacterMovementComponent.h"

// Sets default values
ARidd_Character::ARidd_Character(const class FObjectInitializer& ObjectInitializer) :
// Set the character movement component to use
	Super(ObjectInitializer.SetDefaultSubobjectClass<URidd_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ARidd_Character::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARidd_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ARidd_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

URidd_CharacterMovementComponent* ARidd_Character::GetMyMovementComponent() const
{
	return reinterpret_cast<URidd_CharacterMovementComponent*>(GetCharacterMovement());
}

