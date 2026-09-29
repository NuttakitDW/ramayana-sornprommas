extends CharacterBody3D
## Player character: หนุมาน (Hanuman) with his ตรีเพชร (trident).
## Light combo (swing, swing, overhead slam), heavy thrust, leap + plunge,
## dash, and นิมิตกาย (magical growth, see lore/characters/hanuman.md).

signal died

const Figure := preload("res://scripts/actors/khon_figure.gd")
const Pal := preload("res://scripts/core/palette.gd")
const Combat := preload("res://scripts/combat/combat.gd")
const Bot := preload("res://scripts/actors/hanuman_bot.gd")

const MAX_HP := 140.0
const SPEED := 7.2
const ACCEL := 42.0
const JUMP_V := 10.5
const GRAVITY := 26.0
const DASH_SPEED := 17.0
const DASH_TIME := 0.2
const DASH_COOLDOWN := 0.45
const GIANT_TIME := 9.0
const GIANT_SCALE := 1.7
const POWER_PER_HIT := 7.0

const ATTACKS := {
	"light1": {"pose": "swing_a", "len": 0.36, "hit_at": 0.13, "dmg": 12.0, "reach": 2.6, "arc": 130.0, "knock": 3.0, "lunge": 3.0, "shake": 0.12},
	"light2": {"pose": "swing_b", "len": 0.36, "hit_at": 0.13, "dmg": 13.0, "reach": 2.6, "arc": 130.0, "knock": 3.0, "lunge": 3.0, "shake": 0.12},
	"light3": {"pose": "slam", "len": 0.52, "hit_at": 0.27, "dmg": 24.0, "reach": 2.9, "arc": 160.0, "knock": 9.0, "lunge": 4.0, "shake": 0.35},
	"heavy": {"pose": "thrust", "len": 0.58, "hit_at": 0.27, "dmg": 32.0, "reach": 3.4, "arc": 60.0, "knock": 11.0, "lunge": 10.0, "shake": 0.3},
}

var hp := MAX_HP
var power := 0.0
var state := "normal"
var input_enabled := true
var autobot := false
var giant_left := 0.0

var model: Node3D
var combo_count := 0
var max_combo := 0
var hits_taken := 0

var _attack: Dictionary = {}
var _attack_id := ""
var _attack_t := 0.0
var _attack_hit_done := false
var _combo_step := 0
var _queued := ""
var _dash_t := 0.0
var _dash_cd := 0.0
var _hurt_t := 0.0
var _invuln_t := 0.0
var _special_t := 0.0
var _combo_timer := 0.0
var _facing := 0.0
var _bot: RefCounted
var _aura: OmniLight3D


func _ready() -> void:
	add_to_group("player")
	collision_layer = 2
	collision_mask = 1
	var shape := CollisionShape3D.new()
	var capsule := CapsuleShape3D.new()
	capsule.radius = 0.4
	capsule.height = 1.8
	shape.shape = capsule
	shape.position = Vector3(0, 0.9, 0)
	add_child(shape)

	model = Figure.new()
	add_child(model)
	model.build({
		"kind": "ling", "skin": Pal.HANUMAN_WHITE, "cloth": Pal.KHON_RED,
		"trim": Pal.KHON_RED_DARK, "crown": "hanuman", "weapon": "trident",
		"mouth": "open", "tail": true,
	})
	_aura = OmniLight3D.new()
	_aura.light_color = Color(0.6, 0.8, 1.0)
	_aura.light_energy = 0.0
	_aura.omni_range = 6.0
	_aura.position = Vector3(0, 1.6, 0)
	add_child(_aura)
	_facing = PI
	rotation.y = _facing
	if autobot:
		_bot = Bot.new(self)


func is_alive() -> bool:
	return state != "dead"


func damage_mult() -> float:
	return 1.5 if giant_left > 0.0 else 1.0


func _pressed(action: String) -> bool:
	if not input_enabled:
		return false
	if _bot != null:
		return _bot.pressed(action)
	return Input.is_action_just_pressed(action)


func _move_input() -> Vector2:
	if not input_enabled:
		return Vector2.ZERO
	if _bot != null:
		return _bot.move()
	return Input.get_vector("move_left", "move_right", "move_forward", "move_back")


func _physics_process(delta: float) -> void:
	if _bot != null and input_enabled:
		_bot.think(delta)
	_tick_timers(delta)
	var cam := get_tree().get_first_node_in_group("camera_rig")
	var input := _move_input()
	var wish := Vector3.ZERO
	if cam != null:
		wish = cam.right_flat() * input.x - cam.forward_flat() * input.y
	if wish.length() > 1.0:
		wish = wish.normalized()

	match state:
		"normal": _state_normal(delta, wish)
		"attack": _state_attack(delta)
		"plunge": _state_plunge()
		"dash": _state_dash()
		"special": _state_special(delta)
		_: _decelerate(delta, 10.0)

	if not is_on_floor():
		velocity.y -= GRAVITY * delta
	move_and_slide()
	Combat.clamp_to_arena(self, 0.6)
	rotation.y = lerp_angle(rotation.y, _facing, 1.0 - exp(-18.0 * delta))

	var flat_speed := Vector2(velocity.x, velocity.z).length()
	model.move_ratio = clampf(flat_speed / SPEED, 0.0, 1.0) if state == "normal" else 0.0
	model.grounded = is_on_floor() or state == "down"


func _tick_timers(delta: float) -> void:
	_dash_cd = maxf(0.0, _dash_cd - delta)
	_invuln_t = maxf(0.0, _invuln_t - delta)
	_combo_timer -= delta
	if _combo_timer <= 0.0:
		combo_count = 0
	if state == "hurt":
		_hurt_t -= delta
		if _hurt_t <= 0.0:
			state = "normal"
	if giant_left > 0.0:
		giant_left -= delta
		_aura.light_energy = 1.6 + sin(Time.get_ticks_msec() * 0.01) * 0.4
		if giant_left <= 0.0:
			_set_giant(false)


func _state_normal(delta: float, wish: Vector3) -> void:
	var speed := SPEED * (1.15 if giant_left > 0.0 else 1.0)
	var target := wish * speed
	var h := Vector3(velocity.x, 0, velocity.z).move_toward(target, ACCEL * delta)
	velocity.x = h.x
	velocity.z = h.z
	if wish.length() > 0.1:
		_facing = Combat.yaw_toward(wish)
	if _pressed("jump") and is_on_floor():
		velocity.y = JUMP_V
		_sfx("whoosh", 0.8)
	if _pressed("attack"):
		if is_on_floor():
			_start_attack("light1")
		else:
			_start_plunge()
	elif _pressed("heavy") and is_on_floor():
		_start_attack("heavy")
	elif _pressed("dash") and _dash_cd <= 0.0:
		_start_dash(wish)
	elif _pressed("special") and power >= 100.0 and giant_left <= 0.0:
		_start_special()


func _decelerate(delta: float, rate: float) -> void:
	var h := Vector3(velocity.x, 0, velocity.z).move_toward(Vector3.ZERO, rate * delta * 4.0)
	velocity.x = h.x
	velocity.z = h.z


# ---------------------------------------------------------------- attacks

func _start_attack(id: String) -> void:
	_attack_id = id
	_attack = ATTACKS[id]
	_attack_t = 0.0
	_attack_hit_done = false
	_queued = ""
	state = "attack"
	_auto_face()
	var fwd := -global_transform.basis.z
	var lunge: float = _attack["lunge"]
	velocity.x = fwd.x * lunge
	velocity.z = fwd.z * lunge
	model.play(_attack["pose"], _attack["len"])
	_sfx("whoosh", 1.2 if id.begins_with("light") else 0.9, -4.0)


func _auto_face() -> void:
	var hit := Combat.nearest_point(get_tree(), global_position + Vector3(0, 1, 0), 5.5, 3.0)
	if hit.is_empty():
		return
	var to: Vector3 = hit["point"] - global_position
	_facing = Combat.yaw_toward(Vector3(to.x, 0, to.z))
	rotation.y = _facing


func _state_attack(delta: float) -> void:
	_attack_t += delta
	_decelerate(delta, 9.0)
	if _pressed("attack"):
		_queued = "light"
	elif _pressed("heavy"):
		_queued = "heavy"
	if not _attack_hit_done and _attack_t >= _attack["hit_at"]:
		_attack_hit_done = true
		_resolve_hit(_attack)
	if _attack_hit_done and _pressed("dash"):
		_start_dash(Vector3.ZERO)
		return
	if _attack_t >= _attack["len"]:
		_finish_attack()


func _finish_attack() -> void:
	if _queued == "light" and _attack_id == "plunge":
		_start_attack("light1")
	elif _queued == "light" and _attack_id != "light3" and _attack_id != "heavy":
		_start_attack("light2" if _attack_id == "light1" else "light3")
	elif _queued == "heavy":
		_start_attack("heavy")
	else:
		state = "normal"


func _resolve_hit(atk: Dictionary) -> void:
	var s := GIANT_SCALE if giant_left > 0.0 else 1.0
	var origin := global_position + Vector3(0, 1.0 * s, 0)
	var hits := Combat.sweep(get_tree(), origin, -global_transform.basis.z, atk["reach"] * s, atk["arc"], 2.3 * s)
	for h in hits:
		_apply_hit(h, atk["dmg"] * damage_mult(), atk["knock"], atk["shake"])


func _apply_hit(h: Dictionary, dmg: float, knock: float, shake: float) -> void:
	var landed: bool = h["target"].receive_hit(dmg, global_position, knock, h["index"])
	var fx := get_tree().get_first_node_in_group("fx")
	if landed:
		combo_count += 1
		max_combo = maxi(max_combo, combo_count)
		_combo_timer = 2.2
		power = minf(100.0, power + POWER_PER_HIT)
		if fx:
			fx.sparks(h["point"], Pal.GOLD, 16)
			fx.number(h["point"] + Vector3(0, 0.4, 0), dmg, Pal.UI_PALE)
			fx.hitstop(0.05 + shake * 0.12)
		_sfx("hit", 1.0 + randf() * 0.2)
		_shake(shake)
	elif fx:
		fx.sparks(h["point"], Pal.UI_PALE, 8, 4.0)
		_sfx("clang", 1.4, -6.0)


func _start_plunge() -> void:
	state = "plunge"
	velocity = Vector3(velocity.x * 0.3, -24.0, velocity.z * 0.3)
	model.play("plunge", 0.4, true)
	_sfx("whoosh", 0.7)


func _state_plunge() -> void:
	velocity.y = -24.0
	if is_on_floor():
		var s := GIANT_SCALE if giant_left > 0.0 else 1.0
		var hits := Combat.sweep(get_tree(), global_position + Vector3(0, 0.8, 0), -global_transform.basis.z, 2.8 * s, 360.0, 2.6 * s)
		for h in hits:
			_apply_hit(h, 22.0 * damage_mult(), 8.0, 0.3)
		var fx := get_tree().get_first_node_in_group("fx")
		if fx:
			fx.ring(global_position, 3.2 * s, Color(Pal.GOLD, 0.8))
		_sfx("klong", 1.2)
		_shake(0.3)
		model.clear_action()
		_attack = {"len": 0.22, "hit_at": 99.0}
		_attack_id = "plunge"
		_attack_t = 0.0
		_attack_hit_done = true
		_queued = ""
		state = "attack"


func _start_dash(wish: Vector3) -> void:
	var dir := wish if wish.length() > 0.1 else -global_transform.basis.z
	dir = Vector3(dir.x, 0, dir.z).normalized()
	_facing = Combat.yaw_toward(dir)
	velocity = dir * DASH_SPEED
	_dash_t = DASH_TIME
	_dash_cd = DASH_COOLDOWN
	_invuln_t = DASH_TIME + 0.08
	state = "dash"
	model.clear_action()
	_sfx("whoosh", 1.5, -2.0)


func _state_dash() -> void:
	_dash_t -= get_physics_process_delta_time()
	if _dash_t <= 0.0:
		velocity.x *= 0.3
		velocity.z *= 0.3
		state = "normal"


# ---------------------------------------------------------------- นิมิตกาย

func _start_special() -> void:
	state = "special"
	_special_t = 0.0
	power = 0.0
	velocity = Vector3.ZERO
	model.play("special", 0.7)
	_sfx("klong", 0.7)


func _state_special(delta: float) -> void:
	var before := _special_t
	_special_t += delta
	if before < 0.4 and _special_t >= 0.4:
		_set_giant(true)
		var hits := Combat.sweep(get_tree(), global_position + Vector3(0, 1, 0), Vector3.FORWARD, 7.0, 360.0, 6.0)
		for h in hits:
			_apply_hit(h, 38.0, 13.0, 0.6)
		var fx := get_tree().get_first_node_in_group("fx")
		if fx:
			fx.ring(global_position, 8.0, Color(Pal.UI_SIGNAL, 0.9), 0.6)
	if _special_t >= 0.75:
		state = "normal"


func _set_giant(on: bool) -> void:
	giant_left = GIANT_TIME if on else 0.0
	var tw := create_tween()
	tw.tween_property(model, "scale", Vector3.ONE * (GIANT_SCALE if on else 1.0), 0.35).set_trans(Tween.TRANS_BACK)
	if not on:
		_aura.light_energy = 0.0


# ---------------------------------------------------------------- damage

func take_damage(dmg: float, from_pos: Vector3, knock: float) -> bool:
	if state in ["dead", "down", "cutscene", "special"] or _invuln_t > 0.0 or state == "dash":
		return false
	var giant := giant_left > 0.0
	hp -= dmg * (0.6 if giant else 1.0)
	hits_taken += 1
	combo_count = 0
	model.flash(Pal.DANGER, 0.14)
	_shake(0.4)
	_sfx("hit", 0.7)
	var away := global_position - from_pos
	away.y = 0
	if away.length() > 0.01:
		away = away.normalized()
	if hp <= 0.0:
		hp = 0.0
		state = "dead"
		model.is_down = true
		input_enabled = false
		died.emit()
		return true
	if not giant:
		state = "hurt"
		_hurt_t = 0.32
		_invuln_t = 0.45
		model.play("hurt", 0.3)
		velocity = away * knock + Vector3(0, 2.0, 0)
	return true


# ---------------------------------------------------------------- cutscene API

func enter_cutscene() -> void:
	input_enabled = false
	state = "cutscene"
	model.clear_action()


func leave_cutscene() -> void:
	input_enabled = true
	state = "normal"


func face_point(p: Vector3) -> void:
	var to := p - global_position
	_facing = Combat.yaw_toward(Vector3(to.x, 0, to.z))


func knock_down() -> void:
	state = "down"
	model.is_down = true
	model.play("hurt", 0.3)
	_set_giant(false)


func revive() -> void:
	model.is_down = false
	state = "cutscene"
	hp = maxf(hp, MAX_HP * 0.5)


func _sfx(kind: String, pitch := 1.0, db := 0.0) -> void:
	var m := get_tree().get_first_node_in_group("music")
	if m:
		m.sfx(kind, pitch, db)


func _shake(amount: float) -> void:
	var cam := get_tree().get_first_node_in_group("camera_rig")
	if cam:
		cam.shake(amount)
