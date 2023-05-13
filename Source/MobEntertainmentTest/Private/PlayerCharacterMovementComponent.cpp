#include "PlayerCharacterMovementComponent.h"
#include "PlayerCharacter.h"
#include "Components/CapsuleComponent.h"

#pragma region Saved Move

UPlayerCharacterMovementComponent::FSavedMove_Zippy::FSavedMove_Zippy()
{
	//Saved_bWantsToSprint = 0;
}

bool UPlayerCharacterMovementComponent::FSavedMove_Zippy::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	//const FSavedMove_Zippy* NewZippyMove = static_cast<FSavedMove_Zippy*>(NewMove.Get());

	//if (Saved_bWantsToSprint != NewZippyMove->Saved_bWantsToSprint)
	//{
	//	return false;
	//}

	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UPlayerCharacterMovementComponent::FSavedMove_Zippy::Clear()
{
	FSavedMove_Character::Clear();

	//Saved_bWantsToSprint = 0;
}

uint8 UPlayerCharacterMovementComponent::FSavedMove_Zippy::GetCompressedFlags() const
{
	uint8 Result = FSavedMove_Character::GetCompressedFlags();

	//if (Saved_bWantsToSprint) Result |= FLAG_Sprint;

	return Result;
}

void UPlayerCharacterMovementComponent::FSavedMove_Zippy::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	const UPlayerCharacterMovementComponent* CharacterMovement = Cast<UPlayerCharacterMovementComponent>(C->GetCharacterMovement());

	//Saved_bWantsToSprint = CharacterMovement->Safe_bWantsToSprint;
	Saved_bPrevWantsToCrouch = CharacterMovement->Safe_bPrevWantsToCrouch;
}

void UPlayerCharacterMovementComponent::FSavedMove_Zippy::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);

	UPlayerCharacterMovementComponent* CharacterMovement = Cast<UPlayerCharacterMovementComponent>(C->GetCharacterMovement());

	//CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;
	CharacterMovement->Safe_bPrevWantsToCrouch = Saved_bPrevWantsToCrouch;
}

#pragma endregion

#pragma region Client Network Prediction Data

UPlayerCharacterMovementComponent::FNetworkPredictionData_Client_PlayerCharacter::FNetworkPredictionData_Client_PlayerCharacter(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{

}

FSavedMovePtr UPlayerCharacterMovementComponent::FNetworkPredictionData_Client_PlayerCharacter::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Zippy());
}

#pragma endregion

void UPlayerCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
}

FNetworkPredictionData_Client* UPlayerCharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UPlayerCharacterMovementComponent* MutableThis = const_cast<UPlayerCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_PlayerCharacter(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}
	return ClientPredictionData;
}

void UPlayerCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	//Safe_bWantsToSprint = (Flags & FSavedMove_Zippy::FLAG_Sprint) != 0;
}

void UPlayerCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	//if (MovementMode == MOVE_Walking && !bWantsToCrouch && Safe_bPrevWantsToCrouch)
	//{
	//	FHitResult PotentialSlideSurface;
	//	if (Velocity.SizeSquared() > pow(Slide_MinSpeed, 2) && GetSlideSurface(PotentialSlideSurface))
	//	{
	//		EnterSlide();
	//	}
	//}

	//if (IsCustomMovementMode(CMOVE_Slide) && !bWantsToCrouch)
	//{
	//	SetMovementMode(MOVE_Walking);
	//}

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UPlayerCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	Safe_bPrevWantsToCrouch = bWantsToCrouch;
}

void UPlayerCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CMOVE_Grapple:
		GrappleToSurface(deltaTime, Iterations);
		break;
	default:
		break;
	}
}

void UPlayerCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

}

void UPlayerCharacterMovementComponent::TryGrapple(FVector GrappleLocation)
{
	if (MovementMode != EMovementMode::MOVE_Custom && CustomMovementMode != CMOVE_Grapple)
	{
		GrappleTargetLocation = GrappleLocation;
		Velocity = (GrappleLocation - UpdatedComponent->GetComponentLocation()).GetSafeNormal() * GrappleSpeed;
		SetMovementMode(MOVE_Custom, CMOVE_Grapple);
	}
	else
	{
		//detach from wall?
	}
}

bool UPlayerCharacterMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

void UPlayerCharacterMovementComponent::GrappleToSurface(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	RestorePreAdditiveRootMotionVelocity();

	// Calc Velocity
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		CalcVelocity(deltaTime, Slide_Friction, true, GetMaxBrakingDeceleration());
	}
	ApplyRootMotionToVelocity(deltaTime);

	// Perform Move
	Iterations++;
	bJustTeleported = false;

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FQuat OldRotation = UpdatedComponent->GetComponentRotation().Quaternion();
	FHitResult Hit(1.f);
	FVector Adjusted = Velocity * deltaTime;
	FVector VelPlaneDir = FVector::VectorPlaneProject(Velocity, SurfaceHit.Normal).GetSafeNormal();
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(VelPlaneDir, SurfaceHit.Normal).ToQuat();
	SafeMoveUpdatedComponent(Adjusted, NewRotation, true, Hit);

	if (Hit.Time < 1.f)
	{
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}

	if ((GrappleTargetLocation - UpdatedComponent->GetComponentLocation()).SizeSquared() < (PlayerCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius() + GrappleOffset))
	{
		ExitGrapple();
	}

	// Update Outgoing Velocity & Acceleration
	if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}
}

void UPlayerCharacterMovementComponent::StickToSurface(float deltaTime, int32 Iterations)
{

}

void UPlayerCharacterMovementComponent::ExitGrapple()
{
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(UpdatedComponent->GetForwardVector().GetSafeNormal2D(), FVector::UpVector).ToQuat();
	FHitResult Hit;
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, true, Hit);
	SetMovementMode(MOVE_Falling);
}