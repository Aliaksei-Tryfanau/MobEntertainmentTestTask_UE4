#include "PlayerCharacterMovementComponent.h"
#include "PlayerCharacter.h"
#include "Components/CapsuleComponent.h"

void UPlayerCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
	TargetDistance = PlayerCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius() + GrappleOffset;
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

void UPlayerCharacterMovementComponent::TryGrapple(FVector GrappleLocation)
{
	if (MovementMode != EMovementMode::MOVE_Custom && CustomMovementMode != CMOVE_Grapple)
	{
		GrappleTargetLocation = GrappleLocation;
		Velocity = (GrappleTargetLocation - UpdatedComponent->GetComponentLocation()).GetSafeNormal() * GrappleSpeed;
		SetMovementMode(MOVE_Custom, CMOVE_Grapple);
	}
	else
	{
		//detach from wall?
	}
}

void UPlayerCharacterMovementComponent::GrappleToSurface(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	//UE_LOG(LogTemp, Log, TEXT("Player location %s, targete location %s, target vector: %s"), *UpdatedComponent->GetComponentLocation().ToString(), 
	//	*GrappleTargetLocation.ToString(), *(GrappleTargetLocation - UpdatedComponent->GetComponentLocation()).ToString());
	FVector Direction = GrappleTargetLocation - UpdatedComponent->GetComponentLocation();
	FVector XYDirection = FVector(Direction.X, Direction.Y, 0.f);

	if (XYDirection.SizeSquared() < pow(TargetDistance, 2))
	{
		ExitGrapple();
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	CalcVelocity(deltaTime, 0.f, false, GetMaxBrakingDeceleration());
	Iterations++;
	bJustTeleported = false;
	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FHitResult Hit(1.f);
	FVector Adjusted = Velocity * deltaTime;
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(UpdatedComponent->GetForwardVector().GetSafeNormal2D(), FVector::UpVector).ToQuat();
	SafeMoveUpdatedComponent(Adjusted, NewRotation, true, Hit);

	if (Hit.Time < 1.f)
	{
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}
	else
	{
		Velocity = (GrappleTargetLocation - UpdatedComponent->GetComponentLocation()).GetSafeNormal() * GrappleSpeed;
	}

	Direction = GrappleTargetLocation - UpdatedComponent->GetComponentLocation();
	XYDirection = FVector(Direction.X, Direction.Y, 0.f);
	if (XYDirection.SizeSquared() < pow(TargetDistance, 2))
	{
		ExitGrapple();
	}

	//if (!bJustTeleported)
	//{
	//	Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	//}
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