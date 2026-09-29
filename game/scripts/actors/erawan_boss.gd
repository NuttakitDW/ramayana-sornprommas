extends Node3D
## Boss: อินทรชิต disguised as พระอินทร์, riding the illusory ช้างเอราวัณ.
## Pattern: hover and circle → arrow volley (เชิดฉิ่ง) or diving charge →
## kneel (heads within reach) → rise. Break all three necks to win the fight,
## echoing ร.๒ "ง้างหักฅอพระยาเอราวรรณ".

signal head_broken(index: int, remaining: int)
signal defeated

const Model := preload("res://scripts/actors/erawan_model.gd")
const Pal := preload("res://scripts/core/palette.gd")

const HEAD_HP := 80.0
const HOVER_HEIGHT := 5.5
const ORBIT_RADIUS := 11.0
const CHARGE_SPEED := 21.0
const KNEEL_TIME := 4.2

var head_hp: Array[float] = [HEAD_HP, HEAD_HP, HEAD_HP]
var state := "idle"
## When false the boss only circles (used during story beats).
var active := false
var model: Node3D

var _t := 0.0
var _orbit_a := -PI / 2.0
var _volleys_in_row := 0
var _charge_from := Vector3.ZERO
var _charge_to := Vector3.ZERO
var _charge_hit := false
var _kneel := 0.0


func _ready() -> void:
	add_to_group("hittable")
	add_to_group("boss")
	model = Model.new()
	add_child(model)
	model.build()


func is_alive() -> bool:
	return state != "dead" and state != "idle"


func heads_left() -> int:
	var n := 0
	for v in head_hp:
		if v > 0.0:
			n += 1
	return n


func hurt_points() -> Array:
	var pts: Array = []
	for i in 3:
		if head_hp[i] > 0.0:
			pts.append(model.heads[i].global_position)
		else:
			pts.append(Vector3(0, -1000, 0))
	return pts


func hurt_radius() -> float:
	return 0.9


func receive_hit(dmg: float, _from: Vector3, _knock: float, index: int) -> bool:
	if not is_alive() or index < 0 or index > 2 or head_hp[index] <= 0.0:
		return false
	head_hp[index] -= dmg
	_flash_rider()
	if head_hp[index] <= 0.0:
		head_hp[index] = 0.0
		_break_head(index)
	return true


## Descend from the sky. Called by the director.
func appear() -> void:
	global_position = Vector3(0, 24, -40)
	state = "enter"
	_t = 3.2


func _process(delta: float) -> void:
	_t -= delta
	match state:
		"enter": _enter(delta)
		"hover": _hover(delta)
		"volley": _volley_state(delta)
		"charge_windup": _charge_windup(delta)
		"charge": _charge(delta)
		"kneel": _kneel_state(delta)
		"rise": _rise(delta)
		"dying": pass
	model.set_kneel(_kneel)


func _player() -> Node3D:
	return get_tree().get_first_node_in_group("player")


func _orbit_point() -> Vector3:
	return Vector3(cos(_orbit_a) * ORBIT_RADIUS, HOVER_HEIGHT, sin(_orbit_a) * ORBIT_RADIUS)


func _face_player(delta: float, rate := 4.0) -> void:
	var p := _player()
	if p == null:
		return
	var to := p.global_position - global_position
	rotation.y = lerp_angle(rotation.y, atan2(-to.x, -to.z), 1.0 - exp(-rate * delta))


func _enter(delta: float) -> void:
	global_position = global_position.lerp(_orbit_point(), 1.0 - exp(-1.6 * delta))
	_face_player(delta)
	if _t <= 0.0:
		_go_hover()


func _go_hover() -> void:
	state = "hover"
	_t = randf_range(2.2, 3.2)
	if active:
		_cue("choet_klong")


func _hover(delta: float) -> void:
	_orbit_a += delta * 0.35
	global_position = global_position.lerp(_orbit_point() + Vector3(0, sin(Time.get_ticks_msec() * 0.002) * 0.4, 0), 1.0 - exp(-2.5 * delta))
	_face_player(delta)
	if _t <= 0.0:
		var p := _player()
		if not active or p == null or not p.is_alive():
			_t = 1.0
			return
		if _volleys_in_row >= 1 or randf() < 0.35:
			_start_charge()
		else:
			_start_volley()


# ---------------------------------------------------------------- volley

func _start_volley() -> void:
	state = "volley"
	_t = 2.4
	_volleys_in_row += 1
	model.rider.play("bow_draw", 0.9, true)
	_cue("choet_ching")
	var p := _player()
	var count := 5 + (3 - heads_left()) * 2
	for i in count:
		var offset := Vector3.ZERO if i == 0 else Vector3(randf_range(-1, 1), 0, randf_range(-1, 1)).normalized() * randf_range(1.5, 5.0)
		var at: Vector3 = p.global_position + p.velocity * 0.5 * float(i == 0) + offset
		_schedule_arrow(at, 1.0 + i * 0.08)


func _schedule_arrow(at: Vector3, delay: float) -> void:
	var fx := get_tree().get_first_node_in_group("fx")
	if fx:
		fx.telegraph(at, 1.5, delay, Pal.DANGER)
	var timer := get_tree().create_timer(delay, false)
	timer.timeout.connect(func() -> void: _arrow_land(at))


func _arrow_land(at: Vector3) -> void:
	if state == "dead" or state == "dying":
		return
	var fx := get_tree().get_first_node_in_group("fx")
	if fx:
		fx.sparks(Vector3(at.x, 0.3, at.z), Pal.GOLD, 12, 5.0)
		fx.ring(Vector3(at.x, 0.0, at.z), 1.7, Color(Pal.DANGER, 0.7), 0.3)
	var music := get_tree().get_first_node_in_group("music")
	if music:
		music.sfx("taphon_hi", 1.6, -8.0)
	var p := _player()
	if p != null and Vector2(p.global_position.x - at.x, p.global_position.z - at.z).length() < 1.6 and p.global_position.y < 1.5:
		p.take_damage(13.0, at, 5.0)


func _volley_state(delta: float) -> void:
	_face_player(delta)
	if _t < 1.4 and model.rider.action == "bow_draw":
		model.rider.play("bow_release", 0.5)
	if _t <= 0.0:
		_go_hover()


# ---------------------------------------------------------------- charge

func _start_charge() -> void:
	_volleys_in_row = 0
	state = "charge_windup"
	_t = 1.0
	var music := get_tree().get_first_node_in_group("music")
	if music:
		music.sfx("klong", 0.6, 2.0)


func _charge_windup(delta: float) -> void:
	_face_player(delta, 8.0)
	var p := _player()
	global_position.y = lerpf(global_position.y, 3.0, 1.0 - exp(-3.0 * delta))
	if _t <= 0.0 and p != null:
		_charge_from = global_position
		var target := Vector3(p.global_position.x, 0.0, p.global_position.z)
		var dir := (target - Vector3(_charge_from.x, 0, _charge_from.z)).normalized()
		_charge_to = target + dir * 5.0
		var flat_to := Vector2(_charge_to.x, _charge_to.z)
		if flat_to.length() > 16.0:
			flat_to = flat_to.normalized() * 16.0
			_charge_to = Vector3(flat_to.x, 0, flat_to.y)
		_charge_hit = false
		state = "charge"
		rotation.y = atan2(-dir.x, -dir.z)


func _charge(delta: float) -> void:
	var to := _charge_to - global_position
	var step := CHARGE_SPEED * delta
	if to.length() <= step:
		global_position = _charge_to
		_land()
		return
	global_position += to.normalized() * step
	var p := _player()
	if not _charge_hit and p != null:
		var d := Vector2(p.global_position.x - global_position.x, p.global_position.z - global_position.z).length()
		if d < 2.6 and global_position.y < 3.5:
			_charge_hit = p.take_damage(22.0, global_position, 12.0)


func _land() -> void:
	state = "kneel"
	_t = KNEEL_TIME
	var fx := get_tree().get_first_node_in_group("fx")
	if fx:
		fx.ring(global_position, 6.0, Color(Pal.GOLD, 0.8), 0.5)
	var cam := get_tree().get_first_node_in_group("camera_rig")
	if cam:
		cam.shake(0.5)
	var music := get_tree().get_first_node_in_group("music")
	if music:
		music.sfx("klong", 0.5, 3.0)


func _kneel_state(delta: float) -> void:
	_kneel = move_toward(_kneel, 1.0, delta * 3.0)
	if _t <= 0.0:
		state = "rise"
		_t = 1.2


func _rise(delta: float) -> void:
	_kneel = move_toward(_kneel, 0.0, delta * 2.0)
	global_position = global_position.lerp(_orbit_point(), 1.0 - exp(-1.8 * delta))
	if _t <= 0.0:
		_go_hover()


# ---------------------------------------------------------------- damage

func _break_head(i: int) -> void:
	model.break_head(i)
	var fx := get_tree().get_first_node_in_group("fx")
	if fx:
		var pos: Vector3 = model.heads[i].global_position
		fx.sparks(pos, Pal.GOLD, 40, 10.0)
		fx.ring(Vector3(pos.x, 0, pos.z), 5.0, Color(Pal.UI_SIGNAL, 0.8), 0.5)
		fx.hitstop(0.18)
	var music := get_tree().get_first_node_in_group("music")
	if music:
		music.sfx("klong", 0.45, 4.0)
	var left := heads_left()
	head_broken.emit(i, left)
	if left == 0:
		state = "dying"
		defeated.emit()
	elif state == "kneel":
		_t = minf(_t, 0.6)


func _flash_rider() -> void:
	model.rider.flash(Color.WHITE, 0.08)


func _cue(id: String) -> void:
	var music := get_tree().get_first_node_in_group("music")
	if music:
		music.play_cue(id)
