#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SornHittable.generated.h"

UINTERFACE(meta = (CannotImplementInterfaceInBlueprint))
class USornHittable : public UInterface
{
	GENERATED_BODY()
};

/** Anything Hanuman's trident can strike. */
class SORNPROMMAS_API ISornHittable
{
	GENERATED_BODY()

public:
	virtual bool IsAlive() const = 0;
	/** World-space points that can be struck (e.g. Erawan's three heads). */
	virtual TArray<FVector> HurtPoints() const = 0;
	virtual float HurtRadius() const = 0;
	/** Returns true if damage landed. */
	virtual bool ReceiveHit(float Damage, const FVector& From, float Knock, int32 PointIndex) = 0;
};
