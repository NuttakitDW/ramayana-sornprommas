extends RefCounted
## Builds the head: the Khon mask (หัวโขน) face and its crown (ยอด / ชฎา).
## Visual conventions from lore/khon-performance/05-masks-costumes.md:
##   Hanuman — white, open mouth (ปากอ้า), crystal fangs, earrings (กุณฑล), gold coronet.
##   Yaksha  — bulging eyes (ตาโพลง), fanged grimace, tiered pointed crown.
##   Phra / Indra disguise — green, calm face, slender tall ชฎา with ear ornaments.

const P := preload("res://scripts/actors/khon_parts.gd")
const Pal := preload("res://scripts/core/palette.gd")


static func build_head(head: Node3D, spec: Dictionary) -> void:
	var skin: Color = spec.get("skin", Pal.HANUMAN_WHITE)
	var kind: String = spec.get("kind", "ling")
	var radius := 0.165 if kind == "phra" else 0.19
	P.part(head, P.sphere(radius), P.mat(skin, 0.0, 0.45), Vector3(0, 0.04, 0), Vector3.ZERO, Vector3(1.0, 1.05, 0.98))
	_build_face(head, spec, radius)
	match spec.get("crown", ""):
		"hanuman":
			_crown_hanuman(head)
		"yaksha":
			_crown_yaksha(head, spec.get("trim", Pal.KHON_RED))
		"phra":
			_crown_phra(head)


static func _build_face(head: Node3D, spec: Dictionary, r: float) -> void:
	var kind: String = spec.get("kind", "ling")
	var gold := P.gold()
	var white := P.mat(Pal.EYE_WHITE, 0.0, 0.3)
	var pupil := P.mat(Pal.PUPIL, 0.0, 0.2)
	var fang := P.mat(Pal.FANG, 0.1, 0.2)
	var z := -r + 0.015

	var eye_r := 0.04
	var eye_x := 0.075
	if kind == "yak":
		eye_r = 0.055
		eye_x = 0.085
	elif kind == "phra":
		eye_r = 0.028
		eye_x = 0.06
	for side in [-1.0, 1.0]:
		if kind == "phra":
			P.part(head, P.sphere(eye_r), pupil, Vector3(eye_x * side, 0.06, z + 0.01), Vector3.ZERO, Vector3(1.5, 0.45, 0.6))
		else:
			P.part(head, P.sphere(eye_r), white, Vector3(eye_x * side, 0.07, z + 0.01))
			P.part(head, P.sphere(eye_r * 0.5), pupil, Vector3(eye_x * side, 0.07, z - eye_r * 0.7))
	if kind != "phra":
		P.part(head, P.box(Vector3(0.28, 0.025, 0.04)), gold, Vector3(0, 0.12, z + 0.02), Vector3(0, 0, 0))

	match spec.get("mouth", "calm"):
		"open":
			P.part(head, P.box(Vector3(0.13, 0.07, 0.06)), P.mat(Pal.MOUTH_RED, 0.0, 0.5), Vector3(0, -0.07, z + 0.02))
			for side in [-1.0, 1.0]:
				P.part(head, P.cone(0.012, 0.05, 6), fang, Vector3(0.045 * side, -0.05, z - 0.005), Vector3(180, 0, 0))
				P.part(head, P.cone(0.012, 0.05, 6), fang, Vector3(0.045 * side, -0.095, z - 0.005))
		"grin":
			P.part(head, P.box(Vector3(0.2, 0.045, 0.05)), P.mat(Pal.KHON_RED_DARK, 0.0, 0.5), Vector3(0, -0.075, z + 0.02))
			for side in [-1.0, 1.0]:
				P.part(head, P.cone(0.018, 0.09, 6), fang, Vector3(0.08 * side, -0.04, z - 0.005))
		_:
			P.part(head, P.box(Vector3(0.06, 0.018, 0.03)), P.mat(Pal.MOUTH_RED, 0.0, 0.5), Vector3(0, -0.06, z + 0.02))

	if kind == "ling":
		for side in [-1.0, 1.0]:
			P.part(head, P.torus(0.025, 0.04), gold, Vector3(0.19 * side, -0.02, 0), Vector3(0, 0, 90))


static func _crown_hanuman(head: Node3D) -> void:
	var gold := P.gold()
	P.part(head, P.torus(0.15, 0.2), gold, Vector3(0, 0.14, 0), Vector3(-8, 0, 0), Vector3(1, 1.4, 1))
	P.part(head, P.prism(Vector3(0.12, 0.12, 0.03)), gold, Vector3(0, 0.22, -0.18))
	for side in [-1.0, 1.0]:
		P.part(head, P.prism(Vector3(0.08, 0.1, 0.03)), gold, Vector3(0.12 * side, 0.2, -0.14), Vector3(0, -35.0 * side, 0))


static func _crown_yaksha(head: Node3D, band: Color) -> void:
	var gold := P.gold()
	var band_m := P.mat(band, 0.2, 0.4)
	P.part(head, P.cyl(0.19, 0.2, 0.09), gold, Vector3(0, 0.15, 0))
	P.part(head, P.cyl(0.18, 0.19, 0.03), band_m, Vector3(0, 0.2, 0))
	P.part(head, P.cyl(0.13, 0.17, 0.14), gold, Vector3(0, 0.28, 0))
	P.part(head, P.cyl(0.085, 0.12, 0.14), gold, Vector3(0, 0.41, 0))
	P.part(head, P.cyl(0.05, 0.075, 0.12), gold, Vector3(0, 0.53, 0))
	P.part(head, P.cone(0.045, 0.34, 10), gold, Vector3(0, 0.76, 0))
	for side in [-1.0, 1.0]:
		P.part(head, P.prism(Vector3(0.1, 0.18, 0.03)), gold, Vector3(0.2 * side, 0.02, 0.02), Vector3(0, 90, -20.0 * side))


static func _crown_phra(head: Node3D) -> void:
	var gold := P.gold()
	P.part(head, P.cyl(0.165, 0.17, 0.07), gold, Vector3(0, 0.14, 0))
	for i in 4:
		var y := 0.2 + i * 0.07
		var rr := 0.13 - i * 0.022
		P.part(head, P.cyl(rr, rr + 0.02, 0.06), gold, Vector3(0, y, 0))
	P.part(head, P.cone(0.05, 0.42, 10), gold, Vector3(0, 0.69, 0))
	P.part(head, P.sphere(0.02), P.mat(Pal.UI_SIGNAL, 0.3, 0.2, 2.0), Vector3(0, 0.46, -0.06))
	for side in [-1.0, 1.0]:
		P.part(head, P.prism(Vector3(0.06, 0.22, 0.02)), gold, Vector3(0.17 * side, -0.02, 0.06), Vector3(20, 90, 25.0 * side))
