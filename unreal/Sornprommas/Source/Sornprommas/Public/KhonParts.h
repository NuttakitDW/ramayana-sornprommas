#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "KhonParts.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class USceneComponent;

UENUM()
enum class EKhonShape : uint8
{
	Cube,     // 100 cm cube
	Sphere,   // 100 cm diameter
	Cylinder, // 100 cm diameter, 100 cm tall
	Cone,     // 100 cm base, 100 cm tall, apex +Z
};

/** Per-world cache of engine meshes and dynamic materials used by all primitive figures. */
UCLASS()
class SORNPROMMAS_API UKhonMaterialSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UStaticMesh* Mesh(EKhonShape Shape);

	/** Shared material for a colour/finish; Emissive > 0 makes it glow. */
	UMaterialInterface* Mat(const FLinearColor& Color, float Metallic = 0.f, float Roughness = 0.7f, float Emissive = 0.f);

	/** A material the caller owns (for fading / flashing). */
	UMaterialInstanceDynamic* UniqueMat(UObject* Outer, const FLinearColor& Color, float Emissive = 0.f, bool bTranslucent = false);

	/** Sets colour / alpha on a material from UniqueMat. */
	static void SetMatColor(UMaterialInstanceDynamic* Mat, const FLinearColor& Color, float Opacity = 1.f);

private:
	UMaterialInterface* BaseMaterial(bool bTranslucent);

	UPROPERTY()
	TMap<FString, TObjectPtr<UMaterialInstanceDynamic>> Cache;

	UPROPERTY()
	TMap<uint8, TObjectPtr<UStaticMesh>> Meshes;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> Base;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> BaseTranslucent;

	bool bUsingFallback = false;
};

/** Helpers that assemble figures out of primitive mesh components at runtime. */
namespace KhonParts
{
	/**
	 * Adds a mesh part. Size is in metres along (right, up, back) like the Godot demo:
	 * for Cube it is the box size, for Sphere the diameter per axis, for Cylinder/Cone
	 * (diameter, height, diameter). Location/rotation use Sorn::G / Sorn::RD conventions.
	 */
	SORNPROMMAS_API UStaticMeshComponent* Part(AActor* Owner, USceneComponent* Parent, EKhonShape Shape,
		UMaterialInterface* Material, const FVector& PosM, const FVector& Size, const FVector& RotDeg = FVector::ZeroVector);

	SORNPROMMAS_API USceneComponent* Pivot(AActor* Owner, USceneComponent* Parent, const FVector& PosM, FName Name = NAME_None);

	SORNPROMMAS_API UKhonMaterialSubsystem* Lib(const UObject* WorldContext);

	SORNPROMMAS_API UMaterialInterface* Gold(const UObject* WorldContext);
}
