#include "PlayerCharacter.h"
#include "PlayerCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

APlayerCharacter::APlayerCharacter(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UPlayerCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    PlayerCharacterMovementComponent = Cast<UPlayerCharacterMovementComponent>(GetCharacterMovement());
    PlayerCharacterMovementComponent->SetIsReplicated(true);
	PrimaryActorTick.bCanEverTick = true;

    PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    PlayerCamera->SetupAttachment(RootComponent);
    PlayerCamera->bUsePawnControlRotation = true;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("Forward", this, &APlayerCharacter::MoveForward);
    PlayerInputComponent->BindAxis("Right", this, &APlayerCharacter::MoveRight);
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);
    PlayerInputComponent->BindAxis("Turn", this, &APlayerCharacter::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &APlayerCharacter::LookUp);
    PlayerInputComponent->BindAction("Grapple", IE_Pressed, this, &APlayerCharacter::Grapple);
}

void APlayerCharacter::MoveForward(float Value)
{
    if (Value != 0.0f && PlayerCharacterMovementComponent->MovementMode != MOVE_Custom)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void APlayerCharacter::MoveRight(float Value)
{
    if (Value != 0.0f && PlayerCharacterMovementComponent->MovementMode != MOVE_Custom)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void APlayerCharacter::Turn(float Value)
{
    if (Value != 0.0f)
    {
        AddControllerYawInput(Value * TurnRate * GetWorld()->GetDeltaSeconds());
    }
}

void APlayerCharacter::LookUp(float Value)
{
    if (Value != 0.0f)
    {
        AddControllerPitchInput(Value * LookUpRate * GetWorld()->GetDeltaSeconds());
    }
}

void APlayerCharacter::Grapple()
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!PlayerController)
    {
        return;
    }

    int32 SizeX;
    int32 SizeY;
    PlayerController->GetViewportSize(SizeX, SizeY);
    FVector2D ScreenCenter = FVector2D(SizeX, SizeY) / 2.f;
    FVector WorldLocation, WorldDirection;
    PlayerController->DeprojectScreenPositionToWorld(ScreenCenter.X, ScreenCenter.Y, WorldLocation, WorldDirection);
    FHitResult HitResult;
    const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, WorldLocation + WorldDirection * 10000.f, ECC_Visibility);
    DrawDebugLine(GetWorld(), WorldLocation, WorldLocation + WorldDirection * 10000.f, FColor::Red, false, 10.0f, 0, 1.0f);

    if (bHit)
    {
        PlayerCharacterMovementComponent->TryGrapple(HitResult.Location);
    }
}
