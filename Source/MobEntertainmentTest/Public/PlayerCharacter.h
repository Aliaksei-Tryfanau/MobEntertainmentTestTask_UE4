#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class MOBENTERTAINMENTTEST_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter(const class FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void Grapple();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement) 
	class UPlayerCharacterMovementComponent* PlayerCharacterMovementComponent;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* PlayerCamera;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	float TurnRate = 45.f;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	float LookUpRate = 45.f;

protected:
	virtual void BeginPlay() override;
};
