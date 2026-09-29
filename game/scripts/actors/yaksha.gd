extends CharacterBody3D
## พลยักษ์ — demon foot soldier with a club. Chases Hanuman, waits for an
## attack token from the director, telegraphs a raised-club windup, then strikes.

signal died(enemy: Node)

const Figure := preload("res://scripts/actors/khon_figure.gd")
const Pal := preload("res://scripts/core/palette.gd")
const Combat := preload("res://scripts/combat/combat.gd")

const GRAVITY := 26.0
const STRIKE_RANGE := 2.3
const ORBIT_RANGE := 3.6

var max_hp := 45.0
var hp := 45.0
var speed := 3.6
var damage := 10.0
var windup_time := 0.6
var poise := false
var body_scale := 1.0
var display_name := ""

var state := "enter"
var model: Node3D
var _t := 0.0
var _has_token := false
var _knock := Vector3.ZERO
var _strike_done := false
var _orbit_dir := 1.0


func configure(skin: Color, captain := false) -> void:
	if captain:
		max_hp = 150.0
		speed = 3.2
		damage = 18.0
		windup_time = 0.8
		poise = true
		body_scale = 1.35
	hp = max_hp
	model = Figure.new()
	add_child(model)
	model.build({
		"kind": "yak", "skin": skin, "cloth": Pal.CLOTH_DARK if not captain else Pal.KHON_RED_DARK,
		"trim": Pal.KHON_RED if not captain else Pal.GOLD_DEEP, "crown": "yaksha",
		"weapon": "club", "mouth": "grin",
	})
	model.scale = Vector3.ONE * body_scale


func _ready() -> void:
	add_to_group("hittable")
	add_to_group("enemies")
	collision_layer = 4
	collision_mask = 1
	var shape := CollisionShape3D.new()
	var capsule := CapsuleShape3D.new()
	capsule.radius = 0.45 * body_scale
	capsule.height = 1.9 * body_scale
	shape.shape = capsule
	shape.position = Vector3(0, 0.95 * body_scale, 0)
	add_child(shape)
	_orbit_dir = 1.0 if randf() < 0.5 else -1.0
	_t = randf_range(0.6, 1.2)


func is_alive() -> bool:
	return state != "dead"


func hurt_points() -> Array:
	return [global_position + Vector3(0, 1.1 * body_scale, 0)]


func hurt_radius() -> float:
	return 0.5 * body_scale


func receive_hit(dmg: float, from_pos: Vector3, knock: float, _index: int) -> bool:
	if state == "dead":
		return false
	hp -= dmg
	model.flash(Color.WHITE, 0.1)
	var away := global_position - from_pos
	away.y = 0
	away = away.normalized() if away.length() > 0.01 else Vector3.FORWARD
	_knock = away * knock * (0.4 if poise else 1.0)
	if hp <= 0.0:
		_die()
	elif not (poise and state == "windup"):
		_release_token()
		state = "hurt"
		_t = 0.28 if not poise else 0.12
		model.play("hurt", _t)
	return true


func _die() -> void:
	state = "dead"
	_release_token()
	model.is_down = true
	remove_from_group("hittable")
	died.emit(self)
	var music := get_tree().get_first_node_in_group("music")
	if music:
		music.sfx("klong", 1.3 if body_scale < 1.2 else 0.8)
	var tw := create_tween()
	tw.tween_interval(1.4)
	tw.tween_property(self, "position:y", -2.2, 1.2)
	tw.tween_callback(queue_free)


func _physics_process(delta: float) -> void:
	if state == "dead":
		_knock = _knock.move_toward(Vector3.ZERO, 20.0 * delta)
		velocity = Vector3(_knock.x, velocity.y - GRAVITY * delta, _knock.z)
		move_and_slide()
		return
	var player := get_tree().get_first_node_in_group("player")
	if player == null:
		return
	var to: Vector3 = player.global_position - global_position
	to.y = 0
	var dist := to.length()
	var dir := to / dist if dist > 0.01 else Vector3.FORWARD
	var wish := Vector3.ZERO
	_t -= delta

	match state:
		"enter":
			wish = dir * speed * 1.4
			if _t <= 0.0:
				state = "chase"
		"chase":
			wish = _chase(dir, dist, player)
		"windup":
			_face(dir, delta, 10.0)
			if _t <= 0.0:
				state = "strike"
				_t = 0.45
				_strike_done = false
				model.play("strike", 0.4)
		"strike":
			if not _strike_done and _t <= 0.33:
				_strike_done = true
				_try_hit(player, dist, dir)
			if _t <= 0.0:
				state = "recover"
				_t = 0.7
		"recover":
			if _t <= 0.0:
				_release_token()
				state = "chase"
		"hurt":
			if _t <= 0.0:
				state = "chase"

	wish += _separation() * 2.5
	_knock = _knock.move_toward(Vector3.ZERO, 22.0 * delta)
	var h := Vector3(velocity.x, 0, velocity.z).move_toward(wish, 30.0 * delta) + _knock
	velocity.x = h.x
	velocity.z = h.z
	if not is_on_floor():
		velocity.y -= GRAVITY * delta
	move_and_slide()
	if state != "enter":
		Combat.clamp_to_arena(self, 0.8)
	if state in ["enter", "chase"]:
		_face(dir, delta, 8.0)
	model.move_ratio = clampf(Vector2(velocity.x, velocity.z).length() / speed, 0.0, 1.0) if state in ["enter", "chase"] else 0.0


func _chase(dir: Vector3, dist: float, player: Node) -> Vector3:
	if not player.is_alive():
		return Vector3.ZERO
	if dist <= STRIKE_RANGE and _t <= 0.0:
		if _request_token():
			state = "windup"
			_t = windup_time
			model.play("windup", windup_time, true)
			model.flash(Color(1.0, 0.45, 0.1), windup_time * 0.6)
			return Vector3.ZERO
		_t = 0.4
	if dist > ORBIT_RANGE or _has_token:
		return dir * speed
	# Without a token, circle at a respectful distance (keeps fights readable).
	var tangent := Vector3(-dir.z, 0, dir.x) * _orbit_dir
	return tangent * speed * 0.55 + dir * (dist - ORBIT_RANGE) * 1.5


func _try_hit(player: Node, dist: float, dir: Vector3) -> void:
	var fwd := -global_transform.basis.z
	if dist <= STRIKE_RANGE * body_scale + 0.3 and fwd.angle_to(dir) < deg_to_rad(70.0):
		player.take_damage(damage, global_position, 7.0)
	var music := get_tree().get_first_node_in_group("music")
	if music:
		music.sfx("whoosh", 0.7, -3.0)


func _face(dir: Vector3, delta: float, rate: float) -> void:
	rotation.y = lerp_angle(rotation.y, Combat.yaw_toward(dir), 1.0 - exp(-rate * delta))


func _separation() -> Vector3:
	var push := Vector3.ZERO
	for other in get_tree().get_nodes_in_group("enemies"):
		if other == self or not other.is_alive():
			continue
		var d: Vector3 = global_position - other.global_position
		d.y = 0
		var l := d.length()
		if l < 1.3 and l > 0.001:
			push += d / l * (1.3 - l)
	var player := get_tree().get_first_node_in_group("player")
	if player:
		var d2: Vector3 = global_position - player.global_position
		d2.y = 0
		if d2.length() < 1.0 and d2.length() > 0.001:
			push += d2.normalized() * (1.0 - d2.length()) * 2.0
	return push


func _request_token() -> bool:
	var director := get_tree().get_first_node_in_group("director")
	_has_token = director == null or director.request_token(self)
	return _has_token


func _release_token() -> void:
	if not _has_token:
		return
	_has_token = false
	var director := get_tree().get_first_node_in_group("director")
	if director:
		director.release_token(self)
