#pragma once

#include "CoreMinimal.h"
#include "KhonFigureComponent.h"

/**
 * Builds the head: Khon mask (หัวโขน) face and crown (ยอด / ชฎา).
 * Conventions from lore/khon-performance/05-masks-costumes.md:
 *   Hanuman — white, open mouth (ปากอ้า), fangs, earrings (กุณฑล), gold coronet.
 *   Yaksha  — bulging eyes (ตาโพลง), fanged grimace, tiered pointed crown.
 *   Phra / Indra disguise — calm face, slender tall ชฎา with ear ornaments.
 */
namespace KhonCrowns
{
	void BuildHead(AActor* Owner, USceneComponent* Head, const FKhonSpec& Spec);
}
