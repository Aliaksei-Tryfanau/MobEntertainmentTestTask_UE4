#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerCharacterMovementComponent.generated.h"

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None			UMETA(Hidden),
	CMOVE_Grapple		UMETA(DisplayName = "Grapple"),
	CMOVE_MAX			UMETA(Hidden),
};

UCLASS()
class MOBENTERTAINMENTTEST_API UPlayerCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FSavedMove_Zippy : public FSavedMove_Character
	{
	public:
		enum CompressedFlags
		{
			FLAG_Sprint = 0x10,
			FLAG_Custom_1 = 0x20,
			FLAG_Custom_2 = 0x40,
			FLAG_Custom_3 = 0x80,
		};

		// Flags
		//uint8 Saved_bWantsToSprint : 1;

		// Other Variables
		uint8 Saved_bPrevWantsToCrouch : 1;

		FSavedMove_Zippy();

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FNetworkPredictionData_Client_PlayerCharacter : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_PlayerCharacter(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};

public:
	void TryGrapple(FVector GrappleLocation);

	bool Safe_bPrevWantsToCrouch;

	UFUNCTION(BlueprintPure) 
	bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;

	UPROPERTY(Transient) 
	class APlayerCharacter* PlayerCharacter;

	UPROPERTY(EditDefaultsOnly)
	float GrappleSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly)
	float GrappleMinSpeed = 100.f;

	UPROPERTY(EditDefaultsOnly)
	float GrappleOffset = 5.f;

protected:
	virtual void InitializeComponent() override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

private:
	UPROPERTY()
	FVector GrappleTargetLocation;

	void GrappleToSurface(float deltaTime, int32 Iterations);
	void StickToSurface(float deltaTime, int32 Iterations);
	void ExitGrapple();
};
