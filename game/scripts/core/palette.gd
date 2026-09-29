extends RefCounted
## Colour tokens. Character colours follow lore/khon-performance/05-masks-costumes.md
## (skin colours of the Khon masks); UI colours follow the house identity
## (sapphire + bronze on near-black).

# --- Characters (Khon skin / costume colours) ---
const HANUMAN_WHITE := Color(0.95, 0.94, 0.90)      # หนุมาน กายสีขาว
const HANUMAN_SUIT := Color(0.88, 0.87, 0.84)
const INDRA_GREEN := Color(0.18, 0.50, 0.30)        # พระอินทร์ / อินทรชิตแปลง กายสีเขียว
const ERAWAN_WHITE := Color(0.93, 0.92, 0.88)       # ช้างเอราวัณ ๓ เศียร (ฉบับเวที)
const GOLD := Color(0.86, 0.66, 0.24)
const GOLD_DEEP := Color(0.62, 0.44, 0.14)
const KHON_RED := Color(0.62, 0.10, 0.09)
const KHON_RED_DARK := Color(0.36, 0.06, 0.06)
const MOUTH_RED := Color(0.70, 0.08, 0.10)
const FANG := Color(0.97, 0.95, 0.86)
const EYE_WHITE := Color(0.98, 0.96, 0.90)
const PUPIL := Color(0.05, 0.04, 0.03)
const CLOTH_DARK := Color(0.10, 0.10, 0.16)

## Yaksha foot soldiers (พลยักษ์) wear masks of many colours.
const YAKSHA_SKINS := [
	Color(0.16, 0.42, 0.22),  # เขียว
	Color(0.58, 0.12, 0.10),  # แดง
	Color(0.16, 0.20, 0.46),  # คราม
	Color(0.68, 0.48, 0.14),  # เหลืองหม่น
	Color(0.30, 0.30, 0.34),  # มอหมึก
]

# --- World ---
const EARTH := Color(0.46, 0.36, 0.25)
const EARTH_DARK := Color(0.30, 0.23, 0.16)
const STONE := Color(0.42, 0.40, 0.38)
const SKY_TOP := Color(0.08, 0.13, 0.32)
const SKY_HORIZON := Color(0.86, 0.52, 0.32)
const SUN := Color(1.0, 0.80, 0.58)
const DANGER := Color(0.95, 0.18, 0.10)

# --- UI (house identity, dark ground) ---
const UI_INK := Color("#060810")
const UI_PANEL := Color("#0B0E16")
const UI_EDGE := Color("#1F2740")
const UI_SIGNAL := Color("#5D9BFF")
const UI_DEPTH := Color("#0F2A6B")
const UI_PALE := Color("#E8ECF5")
const UI_ASH := Color("#8A93A8")
const UI_BRONZE := Color("#B08D4F")
const UI_HP := Color("#C8412F")
