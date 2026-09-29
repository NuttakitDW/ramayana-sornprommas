extends RefCounted
## All on-screen story text. Verses are quoted verbatim from public-domain
## sources collected in lore/ (see the `source` line of each entry).

const TITLE := {
	"kicker": "โขน  KHON  /  รามเกียรติ์  RAMAKIEN",
	"title": "ศรพรหมาศ",
	"subtitle": "SORN PHROMMAT  /  HANUMAN BATTLE DEMO",
	"body": [
		"อินทรชิตตั้งพิธีชุบศรพรหมาสตร์  ทัพพระรามรุกประชิดกรุงลงกา",
		"หนุมานทหารเอกนำทัพหน้า",
		"",
		"Indrajit consecrates the Brahmastra arrow while Rama's army presses on Lanka.",
		"Hanuman leads the vanguard.",
	],
	"prompt": "กด ENTER หรือคลิกเพื่อเริ่ม   /   PRESS ENTER OR CLICK TO BEGIN",
}

const MARCH := {
	"lines": ["ทัพพระรามยกออก  หนุมานนำทัพหน้า", "Rama's army marches out with Hanuman at its head"],
	"source": "ปี่พาทย์ทำเพลงกราวนอก  /  KRAW NOK: THE MARCH OF RAMA'S ARMY",
}

const KAMPAN := {
	"lines": ["ตัวกูนี้ชื่อหนุมาน  เป็นทหารพระนารายณ์นาถา", "มึงอย่าอ้างอวดอหังการ์  กูจะฆ่าให้ม้วยบรรลัย"],
	"source": "ร.๑ บทละครเรื่องรามเกียรติ์ สมุดไทยเล่มที่ ๕๓  /  HANUMAN NAMES HIMSELF TO THE DEMON GENERAL KAMPAN",
}

const ERAWAN_APPEARS := {
	"lines": ["อินทรชิตแปลงเป็นพระอินทร์ ทรงช้างเอราวัณ  เทวดานางฟ้าร่ายรำบนฟ้า", "Indrajit, disguised as Indra, rides Erawan across the sky"],
	"source": "ช้างเอราวัณคือการุณราชแปลงกาย  /  THE ELEPHANT IS THE DEMON KARUNARAT TRANSFORMED",
}

const ARROW_LOOSED := {
	"lines": ["พาดสายหมายเขม้นเข่นเขี้ยว  น้าวเหนี่ยวด้วยกำลังอังสา", "สังเกตตรงองค์พระลักษมณ์อนุชา  อสุราก็ลั่นไปทันใด"],
	"source": "ตับพรหมาสตร์ ร้องเพลงเชิดฉิ่ง  /  THE BRAHMASTRA IS LOOSED",
}

const ARMY_FALLS := {
	"lines": ["ลูกศรกระจายดังสายฝน  ตกถูกลิงพลไม่ทนได้", "แล้วต้องพระอนุชาเสนาใน  สลบไปไม่เป็นสมประดี"],
	"source": "ตับพรหมาสตร์ ร้องร่ายรุด  /  LAKSHMANA AND THE ARMY FALL. ONLY HANUMAN STANDS",
}

const LING_LOT := {
	"lines": ["หนุมานไม่ต้องศรศรี  ยืนทะยานดาลโกรธดังอัคคี  ชี้หน้าว่าเหวยสหัสนัยน์", "เหตุใดไปเข้าข้างพวกยักษ์  มาแผลงผลาญพระลักษมณ์ให้ตักษัย"],
	"source": "ตับพรหมาสตร์ เพลงลิงโลด  /  UNTOUCHED BY THE ARROW, HANUMAN RAGES AT THE FALSE INDRA",
}

const NECK_BREAK := {
	"lines": ["ว่าพลางเผ่นโผนโจนทยาน  ขึ้นตีควาญท้ายคชาอาสัญ", "ง้างหักฅอพระยาเอราวรรณ  ชิงคันศรศักดิมัฆวาน"],
	"source": "ร.๒ บทละครเรื่องรามเกียรติ์ เล่ม ๑ บทที่ ๑๒๑๙  /  HE BREAKS ERAWAN'S NECK AND SEIZES THE BOW",
}

const STRUCK_DOWN := {
	"lines": ["หันเหียนเปลี่ยนท่าง่าศรจ้อง  ตีต้องหนุมานชาญชัยศรี", "ตกกระเด็นไปกับเศียรกรี  สลบพับอยู่กับที่ยุทธนา"],
	"source": "ตับพรหมาสตร์ ร้องเพลงลิงลาน  ปี่พาทย์ทำเพลงโอดแหบ  /  STRUCK BY THE BOW, HANUMAN FALLS SENSELESS",
}

const WIND := {
	"kicker": "ลมพระพาย  /  THE WIND OF PHRA PHAI",
	"title": "",
	"body": [
		"หนุมานเป็นบุตรพระพาย  เมื่อลมพัดต้องกายจึงฟื้นคืนสติ",
		"",
		"Hanuman is the son of Phra Phai, god of the wind.",
		"When the wind touches him, he wakes.",
	],
	"source": "โขนพระราชทาน ๒๕๕๒: พระพายพัดมาต้อง  /  ร.๑: พิเภกเป่ามนตร์ให้ฟื้น",
	"prompt": "",
}

const GAME_OVER := {
	"kicker": "ปี่พาทย์ทำเพลงกราวรำพม่า  /  THE DEMONS PREVAIL",
	"title": "หนุมานพ่ายแพ้",
	"subtitle": "HANUMAN HAS FALLEN",
	"body": ["ทัพยักษ์ได้ชัย  แต่ศึกยังไม่จบ", "The demons carry the day, but the war is not over."],
	"prompt": "กด R เพื่อเริ่มใหม่   /   PRESS R TO RETRY",
}


static func end_card(stats: String) -> Dictionary:
	return {
		"kicker": "จบการสาธิต  /  END OF DEMO",
		"title": "ศรพรหมาศ",
		"subtitle": "NEXT: HANUMAN FETCHES THE MEDICINE MOUNTAIN",
		"body": [
			"พระลักษมณ์และพลวานรยังสลบด้วยศรพรหมาสตร์",
			"หนุมานต้องเหาะไปยกภูผาอาวุธมาแก้ศรให้ทันก่อนรุ่งเช้า",
			"",
			"Lakshmana and the army still lie under the Brahmastra.",
			"Hanuman must bring the medicine mountain before dawn.",
		],
		"source": stats,
		"prompt": "กด R เพื่อเล่นอีกครั้ง   /   PRESS R TO PLAY AGAIN",
	}
