#pragma once

#include "CoreMinimal.h"

/** A centre-screen story card. */
struct FSornCard
{
	FString Kicker;
	FString Title;
	FString Subtitle;
	TArray<FString> Body;
	FString Source;
	FString Prompt;
	FString Footer = TEXT("© 2026 Nuttakit Kundum");
};

/** A verse / narration banner shown over gameplay. */
struct FSornBanner
{
	TArray<FString> Lines;
	FString Source;
};

/**
 * All on-screen story text. Verses are quoted verbatim from public-domain
 * sources collected in lore/ (the Source line of each entry names them).
 */
namespace StoryText
{
	inline FSornCard Title()
	{
		FSornCard C;
		C.Kicker = TEXT("โขน  KHON  /  รามเกียรติ์  RAMAKIEN");
		C.Title = TEXT("ศรพรหมาศ");
		C.Subtitle = TEXT("SORN PHROMMAT  /  HANUMAN BATTLE DEMO");
		C.Body = {
			TEXT("อินทรชิตตั้งพิธีชุบศรพรหมาสตร์  ทัพพระรามรุกประชิดกรุงลงกา"),
			TEXT("หนุมานทหารเอกนำทัพหน้า"),
			TEXT(""),
			TEXT("Indrajit consecrates the Brahmastra arrow while Rama's army presses on Lanka."),
			TEXT("Hanuman leads the vanguard."),
		};
		C.Prompt = TEXT("กด ENTER หรือคลิกเพื่อเริ่ม   /   PRESS ENTER OR CLICK TO BEGIN");
		return C;
	}

	inline FSornBanner March()
	{
		return {{TEXT("ทัพพระรามยกออก  หนุมานนำทัพหน้า"), TEXT("Rama's army marches out with Hanuman at its head")},
			TEXT("ปี่พาทย์ทำเพลงกราวนอก  /  KRAW NOK: THE MARCH OF RAMA'S ARMY")};
	}

	inline FSornBanner Kampan()
	{
		return {{TEXT("ตัวกูนี้ชื่อหนุมาน  เป็นทหารพระนารายณ์นาถา"), TEXT("มึงอย่าอ้างอวดอหังการ์  กูจะฆ่าให้ม้วยบรรลัย")},
			TEXT("ร.๑ บทละครเรื่องรามเกียรติ์ สมุดไทยเล่มที่ ๕๓  /  HANUMAN NAMES HIMSELF TO THE DEMON GENERAL KAMPAN")};
	}

	inline FSornBanner ErawanAppears()
	{
		return {{TEXT("อินทรชิตแปลงเป็นพระอินทร์ ทรงช้างเอราวัณ  เทวดานางฟ้าร่ายรำบนฟ้า"), TEXT("Indrajit, disguised as Indra, rides Erawan across the sky")},
			TEXT("ช้างเอราวัณคือการุณราชแปลงกาย  /  THE ELEPHANT IS THE DEMON KARUNARAT TRANSFORMED")};
	}

	inline FSornBanner ArrowLoosed()
	{
		return {{TEXT("พาดสายหมายเขม้นเข่นเขี้ยว  น้าวเหนี่ยวด้วยกำลังอังสา"), TEXT("สังเกตตรงองค์พระลักษมณ์อนุชา  อสุราก็ลั่นไปทันใด")},
			TEXT("ตับพรหมาสตร์ ร้องเพลงเชิดฉิ่ง  /  THE BRAHMASTRA IS LOOSED")};
	}

	inline FSornBanner ArmyFalls()
	{
		return {{TEXT("ลูกศรกระจายดังสายฝน  ตกถูกลิงพลไม่ทนได้"), TEXT("แล้วต้องพระอนุชาเสนาใน  สลบไปไม่เป็นสมประดี")},
			TEXT("ตับพรหมาสตร์ ร้องร่ายรุด  /  LAKSHMANA AND THE ARMY FALL. ONLY HANUMAN STANDS")};
	}

	inline FSornBanner LingLot()
	{
		return {{TEXT("หนุมานไม่ต้องศรศรี  ยืนทะยานดาลโกรธดังอัคคี  ชี้หน้าว่าเหวยสหัสนัยน์"), TEXT("เหตุใดไปเข้าข้างพวกยักษ์  มาแผลงผลาญพระลักษมณ์ให้ตักษัย")},
			TEXT("ตับพรหมาสตร์ เพลงลิงโลด  /  UNTOUCHED BY THE ARROW, HANUMAN RAGES AT THE FALSE INDRA")};
	}

	inline FSornBanner NeckBreak()
	{
		return {{TEXT("ว่าพลางเผ่นโผนโจนทยาน  ขึ้นตีควาญท้ายคชาอาสัญ"), TEXT("ง้างหักฅอพระยาเอราวรรณ  ชิงคันศรศักดิมัฆวาน")},
			TEXT("ร.๒ บทละครเรื่องรามเกียรติ์ เล่ม ๑ บทที่ ๑๒๑๙  /  HE BREAKS ERAWAN'S NECK AND SEIZES THE BOW")};
	}

	inline FSornBanner StruckDown()
	{
		return {{TEXT("หันเหียนเปลี่ยนท่าง่าศรจ้อง  ตีต้องหนุมานชาญชัยศรี"), TEXT("ตกกระเด็นไปกับเศียรกรี  สลบพับอยู่กับที่ยุทธนา")},
			TEXT("ตับพรหมาสตร์ ร้องเพลงลิงลาน  ปี่พาทย์ทำเพลงโอดแหบ  /  STRUCK BY THE BOW, HANUMAN FALLS SENSELESS")};
	}

	inline FSornCard Wind()
	{
		FSornCard C;
		C.Kicker = TEXT("ลมพระพาย  /  THE WIND OF PHRA PHAI");
		C.Body = {
			TEXT("หนุมานเป็นบุตรพระพาย  เมื่อลมพัดต้องกายจึงฟื้นคืนสติ"),
			TEXT(""),
			TEXT("Hanuman is the son of Phra Phai, god of the wind."),
			TEXT("When the wind touches him, he wakes."),
		};
		C.Source = TEXT("โขนพระราชทาน ๒๕๕๒: พระพายพัดมาต้อง  /  ร.๑: พิเภกเป่ามนตร์ให้ฟื้น");
		return C;
	}

	inline FSornCard GameOver()
	{
		FSornCard C;
		C.Kicker = TEXT("ปี่พาทย์ทำเพลงกราวรำพม่า  /  THE DEMONS PREVAIL");
		C.Title = TEXT("หนุมานพ่ายแพ้");
		C.Subtitle = TEXT("HANUMAN HAS FALLEN");
		C.Body = {TEXT("ทัพยักษ์ได้ชัย  แต่ศึกยังไม่จบ"), TEXT("The demons carry the day, but the war is not over.")};
		C.Prompt = TEXT("กด R เพื่อเริ่มใหม่   /   PRESS R TO RETRY");
		return C;
	}

	inline FSornCard EndCard(const FString& Stats)
	{
		FSornCard C;
		C.Kicker = TEXT("จบการสาธิต  /  END OF DEMO");
		C.Title = TEXT("ศรพรหมาศ");
		C.Subtitle = TEXT("NEXT: HANUMAN FETCHES THE MEDICINE MOUNTAIN");
		C.Body = {
			TEXT("พระลักษมณ์และพลวานรยังสลบด้วยศรพรหมาสตร์"),
			TEXT("หนุมานต้องเหาะไปยกภูผาอาวุธมาแก้ศรให้ทันก่อนรุ่งเช้า"),
			TEXT(""),
			TEXT("Lakshmana and the army still lie under the Brahmastra."),
			TEXT("Hanuman must bring the medicine mountain before dawn."),
		};
		C.Source = Stats;
		C.Prompt = TEXT("กด R เพื่อเล่นอีกครั้ง   /   PRESS R TO PLAY AGAIN");
		return C;
	}
}
