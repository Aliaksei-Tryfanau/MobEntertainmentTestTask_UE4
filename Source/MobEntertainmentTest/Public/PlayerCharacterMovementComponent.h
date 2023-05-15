#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerCharacterMovementComponent.generated.h"

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None			UMETA(Hidden),
	CMOVE_Grapple		UMETA(DisplayName = "Grapple"),
	CMOVE_Hanging		UMETA(DisplayName = "Hanging"),
	CMOVE_MAX			UMETA(Hidden),
};

UCLASS()
class MOBENTERTAINMENTTEST_API UPlayerCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(Server, Reliable)
	void TryGrapple(FVector GrappleLocation);
	UFUNCTION(Server, Reliable)
	void TryDisconnect();

	UPROPERTY() 
	class APlayerCharacter* PlayerCharacter;

	UPROPERTY(EditDefaultsOnly, Category = Grapple)
	float GrappleSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = Grapple)
	float GrappleMinSpeed = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = Grapple)
	float GrappleOffset = 10.f;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void InitializeComponent() override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

private:
	UPROPERTY(Replicated)
	FVector GrappleTargetLocation;

	UPROPERTY(Replicated)
	float TargetDistance;

	void GrappleToSurface(float deltaTime, int32 Iterations);
	void ExitGrapple();
};
