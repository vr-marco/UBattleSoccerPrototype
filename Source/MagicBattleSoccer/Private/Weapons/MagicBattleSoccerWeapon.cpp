
#include "MagicBattleSoccerWeapon.h"
#include "MagicBattleSoccer.h"
#include "MagicBattleSoccerGoal.h"
#include "MagicBattleSoccerCharacter.h"
#include "MagicBattleSoccerGameState.h"
#include "MagicBattleSoccerPlayerController.h"

AMagicBattleSoccerWeapon::AMagicBattleSoccerWeapon(const class FObjectInitializer& OI)
	: Super(OI)
{
	MyPawn = nullptr;
	bIsEquipped = false;
	bWantsToFire = false;
	bRefiring = false;
	LocalState = EWeaponState::Idle;
	ServerState = EWeaponState::Idle;
	LastFireTime = -99999.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
}

//////////////////////////////////////////////////////////////////////////
// Replication & effects

void AMagicBattleSoccerWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMagicBattleSoccerWeapon, MyPawn);
	DOREPLIFETIME(AMagicBattleSoccerWeapon, ServerState);
}

void AMagicBattleSoccerWeapon::OnRep_MyPawn()
{
}

//////////////////////////////////////////////////////////////////////////
// Inventory

void AMagicBattleSoccerWeapon::OnEquip(FName InSocketName)
{
	AttachMeshToPawn(InSocketName);

	bIsEquipped = true;

	// Determine the state so that the can reload checks will work
	DetermineWeaponState();
}

void AMagicBattleSoccerWeapon::OnUnEquip()
{
	DetachMeshFromPawn();
	bIsEquipped = false;
	StopFire();

	DetermineWeaponState();
}

//////////////////////////////////////////////////////////////////////////
// Input

/** [local + server] sets the firing target */
void AMagicBattleSoccerWeapon::SetTargetLocation(FVector ThisTargetLocation)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerSetTargetLocation(ThisTargetLocation);
	}

	this->TargetLocation = ThisTargetLocation;
}

/** [local + server] sets the firing target */
void AMagicBattleSoccerWeapon::SetTargetLocationAdjustedForVelocity(FVector ThisTargetLocation, FVector TargetVelocity)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerSetTargetLocationAdjustedForVelocity(ThisTargetLocation, TargetVelocity);
	}

	// Virtual functions for projectile weapons should handle this
	this->TargetLocation = ThisTargetLocation;
}

void AMagicBattleSoccerWeapon::StartFire()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStartFire();
	}

	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}

void AMagicBattleSoccerWeapon::StopFire()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStopFire();
	}

	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

//////////////////////////////////////////////////////////////////////////
// Control

/** check if weapon can fire */
bool AMagicBattleSoccerWeapon::CanFire() const
{
	bool bCanFire = (nullptr != MyPawn) && MyPawn->CanFire();
	bool bStateOKToFire = ((LocalState == EWeaponState::Idle) || (LocalState == EWeaponState::Firing));
	return ((bCanFire == true) && (bStateOKToFire == true));
}

//////////////////////////////////////////////////////////////////////////
// Reading data

EWeaponState AMagicBattleSoccerWeapon::GetCurrentState() const
{
	if (ROLE_Authority == GetLocalRole()
		|| (MyPawn && MyPawn->IsLocallyControlled())
		)
	{
		return LocalState;
	}
	else
	{
		return ServerState;
	}	
}

void AMagicBattleSoccerWeapon::SetOwningPawn(AMagicBattleSoccerCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		SetInstigator(NewOwner);
		MyPawn = NewOwner;
		// net owner for RPC calls
		SetOwner(NewOwner);
	}
}

//////////////////////////////////////////////////////////////////////////
// AI

/** Returns how effective this weapon would be on scene actors in the world's current state */
TArray<FWeaponActorEffectiveness> AMagicBattleSoccerWeapon::GetCurrentEffectiveness()
{
	return TArray<FWeaponActorEffectiveness>();
}

//////////////////////////////////////////////////////////////////////////
// Input - server side

bool AMagicBattleSoccerWeapon::ServerSetTargetLocation_Validate(FVector ThisTargetLocation)
{
	return true;
}

void AMagicBattleSoccerWeapon::ServerSetTargetLocation_Implementation(FVector ThisTargetLocation)
{
	SetTargetLocation(ThisTargetLocation);
}

bool AMagicBattleSoccerWeapon::ServerSetTargetLocationAdjustedForVelocity_Validate(FVector ThisTargetLocation, FVector ThisTargetVelocity)
{
	return true;
}

void AMagicBattleSoccerWeapon::ServerSetTargetLocationAdjustedForVelocity_Implementation(FVector ThisTargetLocation, FVector TargetVelocity)
{
	SetTargetLocationAdjustedForVelocity(ThisTargetLocation, TargetVelocity);
}

bool AMagicBattleSoccerWeapon::ServerStartFire_Validate()
{
	return true;
}

void AMagicBattleSoccerWeapon::ServerStartFire_Implementation()
{
	StartFire();
}

bool AMagicBattleSoccerWeapon::ServerStopFire_Validate()
{
	return true;
}

void AMagicBattleSoccerWeapon::ServerStopFire_Implementation()
{
	StopFire();
}

//////////////////////////////////////////////////////////////////////////
// Weapon usage

bool AMagicBattleSoccerWeapon::ServerHandleFiring_Validate()
{
	return true;
}

void AMagicBattleSoccerWeapon::ServerHandleFiring_Implementation()
{
	HandleFiring();
}

void AMagicBattleSoccerWeapon::HandleFiring()
{
	if (CanFire())
	{
		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			FireWeapon();
		}
	}

	if (nullptr != MyPawn && MyPawn->IsLocallyControlled())
	{
		// local client will notify server
		if (GetLocalRole() < ROLE_Authority)
		{
			ServerHandleFiring();
		}

		// Make the player aim in the direction they're firing if they're not running
		AMagicBattleSoccerPlayerController* const PlayerController = GetInstigator() ? Cast<AMagicBattleSoccerPlayerController>(GetInstigator()->Controller) : nullptr;
		if (nullptr != PlayerController
			&& nullptr != PlayerController->GetPawn()
			&& nullptr != PlayerController->GetPawn()->GetMovementComponent()
			&& PlayerController->GetPawn()->GetMovementComponent()->Velocity.Size() < 0.001f)
		{
			// We can't aim immediately because that could put the camera out of sync with the pawn. Just assign ourselves to the controller and it
			// will take care of things in the next Tick().
			PlayerController->bFaceMouseCursorInTick = true;
		}

		// setup refire timer
		bRefiring = (LocalState == EWeaponState::Firing && WeaponConfig.TimeBetweenShots > 0.0f && WeaponConfig.RepeatingFire);
		if (bRefiring)
		{
			// Start a timer to handle repeating firing
			GetWorldTimerManager().SetTimer(TimerHandle_HandleFiringTimer, this, &AMagicBattleSoccerWeapon::HandleFiring, WeaponConfig.TimeBetweenShots, false);
		}
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}

/** [local] weapon specific fire implementation */
void AMagicBattleSoccerWeapon::FireWeapon()
{
	// Child classes handle this
}

void AMagicBattleSoccerWeapon::SetWeaponState(EWeaponState NewState)
{
	const EWeaponState PrevState = LocalState;

	if (PrevState == EWeaponState::Firing && NewState != EWeaponState::Firing)
	{
		OnBurstFinished();
	}

	LocalState = NewState;

	if (ROLE_Authority == GetLocalRole())
	{
		ServerState = NewState;
	}

	if (PrevState != EWeaponState::Firing && NewState == EWeaponState::Firing)
	{
		OnBurstStarted();
	}
}

void AMagicBattleSoccerWeapon::DetermineWeaponState()
{	
	EWeaponState NewState = EWeaponState::Idle;

	if (nullptr != MyPawn && bIsEquipped)
	{
		if (bWantsToFire && CanFire())
		{			
			NewState = EWeaponState::Firing;
		}
	}

	SetWeaponState(NewState);
}

void AMagicBattleSoccerWeapon::OnEnterInventory(AMagicBattleSoccerCharacter* NewOwner)
{
	SetOwningPawn(NewOwner);
}

void AMagicBattleSoccerWeapon::OnLeaveInventory()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		SetOwningPawn(NULL);
	}

	if (IsAttachedToPawn())
	{
		OnUnEquip();
	}
}

bool AMagicBattleSoccerWeapon::IsEquipped() const
{
	return bIsEquipped;
}

bool AMagicBattleSoccerWeapon::IsAttachedToPawn() const
{
	return bIsEquipped;
}

void AMagicBattleSoccerWeapon::AttachMeshToPawn(FName InSocketName)
{
	if (nullptr != MyPawn)
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// Attach the weapon to the player's right hand
		USkeletalMeshComponent* PlayerMesh = Cast<USkeletalMeshComponent>(MyPawn->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
		AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, InSocketName);
		// Show our mesh
		SetActorHiddenInGame(false);
	}
}

void AMagicBattleSoccerWeapon::DetachMeshFromPawn()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorHiddenInGame(true);
}

void AMagicBattleSoccerWeapon::OnBurstStarted()
{
	// start firing, can be delayed to satisfy TimeBetweenShots
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && WeaponConfig.TimeBetweenShots > 0.0f &&
		LastFireTime + WeaponConfig.TimeBetweenShots > GameTime)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HandleFiringTimer, this, &AMagicBattleSoccerWeapon::HandleFiring, LastFireTime + WeaponConfig.TimeBetweenShots - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void AMagicBattleSoccerWeapon::OnBurstFinished()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiringTimer);
	bRefiring = false;
}
