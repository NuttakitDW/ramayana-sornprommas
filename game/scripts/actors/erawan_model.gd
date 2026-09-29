extends Node3D
## Visual model of ช้างเอราวัณ as staged in Khon: three white heads (the text
## says 33; the stage prop has 3), gold เทริด on each head, red caparison, a
## gilded บุษบก (pavilion) and the rider — อินทรชิต disguised as พระอินทร์.
## The elephant itself is the demon การุณราช transformed.

const P := preload("res://scripts/actors/khon_parts.gd")
const Pal := preload("res://scripts/core/palette.gd")
const Figure := preload("res://scripts/actors/khon_figure.gd")

var body: Node3D
var heads: Array[Node3D] = []
var trunks: Array = []
var rider: Node3D
var legs: Array[Node3D] = []
var _time := 0.0


func build() -> void:
	var white := P.mat(Pal.ERAWAN_WHITE, 0.0, 0.5)
	var gold := P.gold()
	var red := P.mat(Pal.KHON_RED, 0.1, 0.5)

	body = P.pivot(self, Vector3(0, 2.2, 0), "Body")
	P.part(body, P.capsule(1.3, 4.3), white, Vector3.ZERO, Vector3(90, 0, 0), Vector3(1.0, 1.0, 1.05))
	# Caparison (ผ้าปูหลัง) with gold border.
	P.part(body, P.box(Vector3(2.75, 0.1, 2.7)), red, Vector3(0, 1.28, 0.1))
	P.part(body, P.box(Vector3(2.85, 0.06, 2.8)), gold, Vector3(0, 1.23, 0.1))
	for side in [-1.0, 1.0]:
		P.part(body, P.box(Vector3(0.06, 1.2, 2.5)), red, Vector3(1.36 * side, 0.65, 0.1))
		P.part(body, P.box(Vector3(0.07, 0.12, 2.6)), gold, Vector3(1.37 * side, 0.06, 0.1))
	# Legs with gold anklets.
	for x in [-0.75, 0.75]:
		for z in [-1.35, 1.35]:
			var leg := P.pivot(body, Vector3(x, -0.6, z), "Leg")
			P.part(leg, P.cyl(0.36, 0.42, 2.0, 12), white, Vector3(0, -0.9, 0))
			P.part(leg, P.torus(0.36, 0.48), gold, Vector3(0, -1.7, 0), Vector3.ZERO, Vector3(1, 0.7, 1))
			legs.append(leg)
	# Tail.
	P.part(body, P.cyl(0.04, 0.08, 1.2, 6), white, Vector3(0, 0.2, 2.55), Vector3(-160, 0, 0))

	for i in 3:
		heads.append(_build_head(i, white, gold))
	_build_pavilion(gold, red)


func _build_head(i: int, white: Material, gold: Material) -> Node3D:
	var x := (i - 1) * 1.05
	var head := P.pivot(body, Vector3(x, 0.55, -2.35), "Head%d" % i)
	head.rotation.y = -(i - 1) * 0.3
	P.part(head, P.sphere(0.72), white, Vector3.ZERO, Vector3.ZERO, Vector3(1.0, 1.05, 1.1))
	# Ears.
	for side in [-1.0, 1.0]:
		P.part(head, P.cyl(0.55, 0.55, 0.06, 16), white, Vector3(0.62 * side, 0.05, 0.2), Vector3(0, 0, 90), Vector3(1, 1, 1.2))
	# เทริด crown.
	P.part(head, P.cyl(0.36, 0.42, 0.18, 12), gold, Vector3(0, 0.62, -0.05))
	P.part(head, P.cone(0.3, 0.55, 12), gold, Vector3(0, 0.98, -0.05))
	P.part(head, P.box(Vector3(0.5, 0.35, 0.05)), gold, Vector3(0, 0.25, -0.73))
	# Eyes.
	for side in [-1.0, 1.0]:
		P.part(head, P.sphere(0.06), P.mat(Pal.PUPIL), Vector3(0.32 * side, 0.12, -0.6))
		# Tusks (งา).
		P.part(head, P.cone(0.07, 0.8, 8), P.mat(Pal.FANG, 0.1, 0.25), Vector3(0.24 * side, -0.4, -0.85), Vector3(-65, 0, 0))
	# Trunk (งวง) as a chain of segments.
	var parent := P.pivot(head, Vector3(0, -0.25, -0.72))
	var segs: Array[Node3D] = []
	for s in 6:
		var seg := P.pivot(parent, Vector3(0, 0.0 if s == 0 else -0.28, 0))
		P.part(seg, P.capsule(0.19 - s * 0.022, 0.34), white, Vector3(0, -0.14, 0))
		segs.append(seg)
		parent = seg
	trunks.append(segs)
	return head


## บุษบก pavilion with the rider standing inside.
func _build_pavilion(gold: Material, red: Material) -> void:
	var base := P.pivot(body, Vector3(0, 1.35, 0.2), "Pavilion")
	P.part(base, P.box(Vector3(1.7, 0.18, 1.7)), gold)
	for x in [-0.72, 0.72]:
		for z in [-0.72, 0.72]:
			P.part(base, P.cyl(0.05, 0.05, 2.1, 8), gold, Vector3(x, 1.05, z))
	var roof_y := 2.1
	for t in 4:
		var w := 1.9 - t * 0.38
		P.part(base, P.box(Vector3(w, 0.16, w)), red if t % 2 == 1 else gold, Vector3(0, roof_y + t * 0.2, 0))
	P.part(base, P.cone(0.28, 1.4, 12), gold, Vector3(0, roof_y + 1.45, 0))

	rider = Figure.new()
	base.add_child(rider)
	rider.position = Vector3(0, 0.1, -0.1)
	rider.build({
		"kind": "phra", "skin": Pal.INDRA_GREEN, "cloth": Pal.GOLD_DEEP,
		"trim": Pal.KHON_RED, "crown": "phra", "weapon": "bow", "mouth": "calm",
	})


func _process(delta: float) -> void:
	_time += delta
	for i in trunks.size():
		var segs: Array = trunks[i]
		for s in segs.size():
			var seg: Node3D = segs[s]
			seg.rotation.x = (0.18 if s > 0 else 0.35) + sin(_time * 1.8 + i + s * 0.4) * 0.12
			seg.rotation.z = sin(_time * 1.3 + i * 2.0 + s * 0.3) * 0.08


## Folds the legs for the kneeling pose (0 = standing, 1 = kneeling).
func set_kneel(amount: float) -> void:
	body.position.y = lerpf(2.2, 1.35, amount)
	body.rotation.x = lerpf(0.0, -0.14, amount)
	for leg in legs:
		leg.rotation.x = lerpf(0.0, -1.2 if leg.position.z < 0.0 else 1.2, amount) * 0.6


func break_head(i: int) -> void:
	var head := heads[i]
	var tw := create_tween()
	tw.tween_property(head, "rotation:x", 1.2, 0.25).set_trans(Tween.TRANS_BACK)
	tw.tween_property(head, "scale", Vector3(0.05, 0.05, 0.05), 0.35)
	tw.tween_callback(head.hide)
