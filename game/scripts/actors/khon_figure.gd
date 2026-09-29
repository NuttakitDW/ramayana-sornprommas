extends Node3D
## A Khon-style figure assembled from primitives, with a small pivot rig that is
## animated procedurally: Khon stance with the ยืดยุบ (rise-and-sink) bounce on
## the music beat, a run cycle, a flying pose (ท่าเหาะ) and action poses.
##
## Spec keys:
##   kind: "ling" | "yak" | "phra"    (Khon character type: monkey / demon / refined)
##   skin, cloth, trim: Color
##   crown: "hanuman" | "yaksha" | "phra"
##   weapon: "trident" | "club" | "bow" | ""
##   mouth: "open" | "grin" | "calm"
##   tail: bool

const P := preload("res://scripts/actors/khon_parts.gd")
const Pal := preload("res://scripts/core/palette.gd")
const Crowns := preload("res://scripts/actors/khon_crowns.gd")
const Poses := preload("res://scripts/actors/khon_poses.gd")

const HIP_Y := 0.98
const POSE_SHARPNESS := 14.0

var spec: Dictionary = {}

var root_pivot: Node3D
var hips: Node3D
var chest: Node3D
var head: Node3D
var arm_l: Node3D
var arm_r: Node3D
var fore_l: Node3D
var fore_r: Node3D
var hand_l: Node3D
var hand_r: Node3D
var weapon_pivot: Node3D
var leg_l: Node3D
var leg_r: Node3D
var shin_l: Node3D
var shin_r: Node3D
var tail_segments: Array[Node3D] = []

var move_ratio := 0.0
var grounded := true
var is_down := false

var action := ""
var action_t := 0.0
var action_len := 0.0
var action_hold := false

var _time := 0.0
var _run_phase := 0.0
var _meshes: Array[MeshInstance3D] = []
var _flash_left := 0.0
var _flash_mat: StandardMaterial3D
var _music: Node = null


func build(figure_spec: Dictionary) -> void:
	spec = figure_spec
	var kind: String = spec.get("kind", "ling")
	var skin: Color = spec.get("skin", Pal.HANUMAN_WHITE)
	var cloth: Color = spec.get("cloth", Pal.KHON_RED)
	var trim: Color = spec.get("trim", Pal.KHON_RED_DARK)
	var gold := P.gold()
	var skin_m := P.mat(skin, 0.0, 0.55)
	var cloth_m := P.mat(cloth, 0.1, 0.5)
	var trim_m := P.mat(trim, 0.1, 0.5)

	root_pivot = P.pivot(self, Vector3.ZERO, "Root")
	hips = P.pivot(root_pivot, Vector3(0, HIP_Y, 0), "Hips")

	# --- Lower costume: ผ้านุ่ง, ห้อยหน้า, หางหงส์, belt ---
	P.part(hips, P.box(Vector3(0.36, 0.2, 0.24)), cloth_m, Vector3(0, 0.02, 0))
	P.part(hips, P.cyl(0.21, 0.27, 0.32), cloth_m, Vector3(0, -0.12, 0))
	P.part(hips, P.torus(0.19, 0.23), gold, Vector3(0, 0.1, 0), Vector3.ZERO, Vector3(1, 0.6, 1))
	P.part(hips, P.box(Vector3(0.15, 0.46, 0.02)), trim_m, Vector3(0, -0.2, -0.26))
	P.part(hips, P.box(Vector3(0.17, 0.48, 0.015)), gold, Vector3(0, -0.2, -0.25))
	P.part(hips, P.box(Vector3(0.26, 0.5, 0.02)), trim_m, Vector3(0, -0.16, 0.27), Vector3(-14, 0, 0))

	# --- Chest: suit, กรองคอ, อินทรธนู, สังวาล, ทับทรวง ---
	chest = P.pivot(hips, Vector3(0, 0.12, 0), "Chest")
	P.part(chest, P.capsule(0.21, 0.62), skin_m, Vector3(0, 0.26, 0), Vector3.ZERO, Vector3(1.08, 1, 0.8))
	P.part(chest, P.cyl(0.25, 0.27, 0.05), gold, Vector3(0, 0.52, 0))
	P.part(chest, P.cyl(0.2, 0.22, 0.055), trim_m, Vector3(0, 0.53, 0))
	P.part(chest, P.box(Vector3(0.05, 0.62, 0.02)), gold, Vector3(0.02, 0.26, -0.175), Vector3(0, 0, 32))
	P.part(chest, P.box(Vector3(0.05, 0.62, 0.02)), gold, Vector3(-0.02, 0.26, -0.175), Vector3(0, 0, -32))
	P.part(chest, P.prism(Vector3(0.12, 0.14, 0.03)), gold, Vector3(0, 0.34, -0.19), Vector3(0, 0, 180))
	for side in [-1.0, 1.0]:
		P.part(chest, P.prism(Vector3(0.16, 0.22, 0.12)), gold, Vector3(0.29 * side, 0.56, 0), Vector3(0, 0, -28.0 * side))

	# --- Head + crown ---
	head = P.pivot(chest, Vector3(0, 0.62, 0), "Head")
	Crowns.build_head(head, spec)

	# --- Arms ---
	var arms := _build_arm(-1.0, skin_m, gold)
	arm_l = arms[0]; fore_l = arms[1]; hand_l = arms[2]
	arms = _build_arm(1.0, skin_m, gold)
	arm_r = arms[0]; fore_r = arms[1]; hand_r = arms[2]
	weapon_pivot = P.pivot(hand_r, Vector3.ZERO, "Weapon")
	_build_weapon(spec.get("weapon", ""))

	# --- Legs (สนับเพลา) ---
	var legs := _build_leg(-1.0, cloth_m, gold, skin_m)
	leg_l = legs[0]; shin_l = legs[1]
	legs = _build_leg(1.0, cloth_m, gold, skin_m)
	leg_r = legs[0]; shin_r = legs[1]

	if spec.get("tail", false):
		_build_tail(skin_m)

	if kind == "yak":
		hips.scale = Vector3(1.12, 1.0, 1.1)

	_meshes = P.collect_meshes(self)
	_flash_mat = StandardMaterial3D.new()
	_flash_mat.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	_flash_mat.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
	_flash_mat.albedo_color = Color(1, 1, 1, 0.0)


func _build_arm(side: float, skin_m: Material, gold: Material) -> Array:
	var shoulder := P.pivot(chest, Vector3(0.28 * side, 0.46, 0), "Arm")
	P.part(shoulder, P.capsule(0.068, 0.36), skin_m, Vector3(0, -0.16, 0))
	var elbow := P.pivot(shoulder, Vector3(0, -0.31, 0), "Fore")
	P.part(elbow, P.capsule(0.058, 0.33), skin_m, Vector3(0, -0.15, 0))
	P.part(elbow, P.torus(0.05, 0.075), gold, Vector3(0, -0.24, 0))
	var hand := P.pivot(elbow, Vector3(0, -0.3, 0), "Hand")
	P.part(hand, P.sphere(0.06), skin_m)
	return [shoulder, elbow, hand]


func _build_leg(side: float, cloth_m: Material, gold: Material, skin_m: Material) -> Array:
	var hip_joint := P.pivot(hips, Vector3(0.12 * side, -0.06, 0), "Leg")
	P.part(hip_joint, P.capsule(0.09, 0.46), cloth_m, Vector3(0, -0.21, 0))
	var knee := P.pivot(hip_joint, Vector3(0, -0.44, 0), "Shin")
	P.part(knee, P.torus(0.07, 0.1), gold, Vector3(0, -0.02, 0))
	P.part(knee, P.capsule(0.07, 0.44), skin_m, Vector3(0, -0.21, 0))
	var foot := P.pivot(knee, Vector3(0, -0.43, 0), "Foot")
	P.part(foot, P.box(Vector3(0.1, 0.06, 0.22)), skin_m, Vector3(0, -0.02, -0.05))
	return [hip_joint, knee]


func _build_tail(skin_m: Material) -> void:
	var parent: Node3D = P.pivot(hips, Vector3(0, -0.05, 0.22), "Tail")
	for i in 7:
		var seg := P.pivot(parent, Vector3(0, 0.0 if i == 0 else 0.12, 0))
		P.part(seg, P.capsule(0.035 - i * 0.003, 0.16), skin_m, Vector3(0, 0.06, 0))
		tail_segments.append(seg)
		parent = seg
	tail_segments[0].rotation.x = 1.9


func _build_weapon(kind: String) -> void:
	var gold := P.gold()
	match kind:
		"trident":
			# ตรีเพชร — Hanuman's trident. Prongs point along +Y of the weapon pivot.
			P.part(weapon_pivot, P.cyl(0.024, 0.024, 1.7, 8), P.mat(Pal.KHON_RED_DARK, 0.3, 0.4), Vector3(0, 0.35, 0))
			P.part(weapon_pivot, P.sphere(0.05), gold, Vector3(0, 1.2, 0))
			P.part(weapon_pivot, P.cone(0.045, 0.42, 8), gold, Vector3(0, 1.43, 0))
			for side in [-1.0, 1.0]:
				P.part(weapon_pivot, P.cyl(0.018, 0.018, 0.16, 6), gold, Vector3(0.07 * side, 1.23, 0), Vector3(0, 0, -60.0 * side))
				P.part(weapon_pivot, P.cone(0.035, 0.3, 8), gold, Vector3(0.13 * side, 1.36, 0))
			P.part(weapon_pivot, P.cone(0.03, 0.12, 8), gold, Vector3(0, -0.56, 0), Vector3(180, 0, 0))
		"club":
			# กระบอง — foot soldier club.
			P.part(weapon_pivot, P.cyl(0.075, 0.04, 1.0, 10), P.mat(Pal.KHON_RED_DARK, 0.2, 0.5), Vector3(0, 0.38, 0))
			for y in [0.12, 0.5, 0.84]:
				P.part(weapon_pivot, P.cyl(0.07, 0.07, 0.035, 10), gold, Vector3(0, y, 0))
		"bow":
			# ธนู — the bow of Indra the disguised Indrajit carries.
			var bow := P.pivot(hand_l, Vector3.ZERO, "Bow")
			for i in 7:
				var a := deg_to_rad(-60.0 + i * 20.0)
				var seg := P.part(bow, P.cyl(0.02, 0.02, 0.2, 6), gold, Vector3(0, sin(a) * 0.55, -cos(a) * 0.18 + 0.1))
				seg.rotation.x = a * 0.35
			bow.rotation.x = -PI / 2.0


## Starts an action pose. `hold` keeps the final frame until another action.
func play(action_name: String, duration: float, hold := false) -> void:
	action = action_name
	action_t = 0.0
	action_len = maxf(duration, 0.01)
	action_hold = hold


func clear_action() -> void:
	action = ""


func flash(color: Color, duration := 0.12) -> void:
	_flash_mat.albedo_color = Color(color.r, color.g, color.b, 0.75)
	_flash_left = duration
	for m in _meshes:
		m.material_overlay = _flash_mat


func _process(delta: float) -> void:
	_time += delta
	if action != "":
		action_t += delta
		if action_t >= action_len and not action_hold:
			action = ""
	_update_flash(delta)
	var pose := _compute_pose(delta)
	_apply_pose(pose, delta)


func _update_flash(delta: float) -> void:
	if _flash_left <= 0.0:
		return
	_flash_left -= delta
	if _flash_left <= 0.0:
		for m in _meshes:
			m.material_overlay = null


func _beat_phase() -> float:
	if _music == null:
		_music = get_tree().get_first_node_in_group("music")
	if _music != null:
		return _music.beat_phase
	return fmod(_time * 1.2, 1.0)


func _compute_pose(delta: float) -> Dictionary:
	var ctx := {
		"kind": spec.get("kind", "ling"),
		"time": _time,
		"beat": _beat_phase(),
		"move": move_ratio,
		"grounded": grounded,
		"action": action,
		"u": clampf(action_t / action_len, 0.0, 1.0) if action != "" else 0.0,
	}
	if move_ratio > 0.05 and grounded:
		_run_phase += delta * (7.0 + 6.0 * move_ratio)
	ctx["run_phase"] = _run_phase
	return Poses.compute(ctx)


func _apply_pose(pose: Dictionary, delta: float) -> void:
	var k := 1.0 - exp(-POSE_SHARPNESS * delta)
	var snap := 1.0 - exp(-40.0 * delta)
	var w := snap if action != "" else k
	for key in pose:
		var node: Node3D = _node_for(key)
		if node == null:
			continue
		var target: Vector3 = pose[key]
		if key == "hips_offset":
			hips.position = hips.position.lerp(Vector3(0, HIP_Y, 0) + target, k)
		else:
			node.rotation = node.rotation.lerp(target, w)
	var down_target := Vector3(-1.45, 0, 0) if is_down else Vector3.ZERO
	root_pivot.rotation = root_pivot.rotation.lerp(down_target, 1.0 - exp(-6.0 * delta))
	root_pivot.position = root_pivot.position.lerp(Vector3(0, 0.25 if is_down else 0.0, 0.4 if is_down else 0.0), 1.0 - exp(-6.0 * delta))
	_animate_tail()


func _node_for(key: String) -> Node3D:
	match key:
		"chest": return chest
		"head": return head
		"arm_l": return arm_l
		"arm_r": return arm_r
		"fore_l": return fore_l
		"fore_r": return fore_r
		"leg_l": return leg_l
		"leg_r": return leg_r
		"shin_l": return shin_l
		"shin_r": return shin_r
		"weapon": return weapon_pivot
		"hips_offset": return hips
	return null


func _animate_tail() -> void:
	for i in range(1, tail_segments.size()):
		var seg := tail_segments[i]
		seg.rotation.x = -0.32 + sin(_time * 3.0 + i * 0.6) * 0.08
		seg.rotation.z = sin(_time * 2.2 + i * 0.5) * 0.12
