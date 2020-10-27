#pragma once

#include "Engine/TargetPoint.h"
#include "MagicBattleSoccerSpawnPoint.generated.h"

/**
*
*/
UCLASS()
class MAGICBATTLESOCCER_API AMagicBattleSoccerSpawnPoint : public ATargetPoint
{
	GENERATED_UCLASS_BODY()

	/** The team number to assign to the spawned soccer player */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Soccer)
	int32 OwningTeamNumber;

	/** True if this is the spawn point for spawning human players who are on the owning team */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Soccer)
	bool SpawnHumanPlayers;

	/** True if this is the spawn point for soccer balls when a round begins */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Soccer)
	bool SpawnSoccerBalls;

	/** The zone that the spawned player is restricted to */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Soccer)
	class ATriggerBox* ActionZone;

	/** The maximum number of bots to spawn on the field */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Soccer)
	int32 MaxBotsOnField;

	/** Called by the GameMode object when the next round has begun */
	UFUNCTION(BlueprintNativeEvent, Category = Soccer)
	void RoundHasStarted();

	/** Called by the GameMode object when a spawned player is about to be destroyed */
	UFUNCTION(BlueprintNativeEvent, Category = Soccer)
	void SpawnedPlayerBeingDestroyed(class AMagicBattleSoccerAIController* Controller);
};
