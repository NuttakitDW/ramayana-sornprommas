#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArenaBuilder.generated.h"

/**
 * Builds the battlefield before Lanka (สนามรบหน้ากรุงลงกา) at dusk: sun, sky,
 * fog, ground, a gilded stage ring, battle standards, torches and the
 * silhouette of Lanka on the northern horizon (+X).
 */
UCLASS()
class SORNPROMMAS_API AArenaBuilder : public AActor
{
	GENERATED_BODY()

public:
	AArenaBuilder();
	virtual void BeginPlay() override;

private:
	void Environment();
	void Ground();
	void StageRing();
	void Standards();
	void Torches();
	void Rocks();
	void LankaSkyline();
	void Hills();
};
