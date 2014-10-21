
#include "MagicBattleSoccer.h"
#include "MagicBattleSoccerGoal.h"
#include "MagicBattleSoccerGameMode.h"
#include "MagicBattleSoccerPlayer.h"

AMagicBattleSoccerGoal::AMagicBattleSoccerGoal(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
}

/** Gets the ideal point for a player to run to when approaching the goal */
FVector AMagicBattleSoccerGoal::GetIdealRunLocation(AMagicBattleSoccerPlayer* Player)
{
	FVector GoalLocation = GetActorLocation();
	FVector FrontOfGoal = GoalLocation + GetActorRightVector() * 570.0f;
	return FVector(FrontOfGoal.X, FrontOfGoal.Y, Player->GetActorLocation().Z);
}

/** This occurs when play begins */
void AMagicBattleSoccerGoal::BeginPlay()
{
	Super::BeginPlay();

	// Add ourselves to the game mode cache
	AMagicBattleSoccerGameMode* GameMode = Cast<AMagicBattleSoccerGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (1 == TeamNumber)
	{
		GameMode->Team1Goal = this;
	}
	else if (2 == TeamNumber)
	{
		GameMode->Team2Goal = this;
	}
}