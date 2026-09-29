extends Node3D
## Third-person orbit camera. Mouse / right stick / arrow keys orbit; when a
## boss is in focus the camera pulls back and frames both fighters.

const MOUSE_SENS := 0.0032
const KEY_SPEED := 2.2

var target: Node3D
var focus: Node3D
var capture_mouse := true
var yaw := 0.0
var pitch := -0.3
var distance := 7.0

var cam: Camera3D
var _look := Vector3.ZERO
var _trauma := 0.0
var _dist := 7.0


func _ready() -> void:
	add_to_group("camera_rig")
	cam = Camera3D.new()
	cam.fov = 60.0
	cam.far = 600.0
	add_child(cam)
	cam.current = true
	if capture_mouse:
		Input.mouse_mode = Input.MOUSE_MODE_CAPTURED


func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseMotion and Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
		yaw -= event.relative.x * MOUSE_SENS
		pitch = clampf(pitch - event.relative.y * MOUSE_SENS * 0.6, -0.85, 0.05)
	elif event.is_action_pressed("release_mouse"):
		Input.mouse_mode = Input.MOUSE_MODE_VISIBLE
	elif event is InputEventMouseButton and event.pressed and capture_mouse:
		Input.mouse_mode = Input.MOUSE_MODE_CAPTURED


func shake(amount: float) -> void:
	_trauma = minf(1.0, _trauma + amount)


func forward_flat() -> Vector3:
	return Vector3(-sin(yaw), 0, -cos(yaw))


func right_flat() -> Vector3:
	return Vector3(cos(yaw), 0, -sin(yaw))


func _process(delta: float) -> void:
	if target == null:
		return
	var real_dt := delta / maxf(Engine.time_scale, 0.001)
	yaw += Input.get_axis("cam_right", "cam_left") * KEY_SPEED * real_dt

	var look := target.global_position + Vector3(0, 1.4, 0)
	var want_dist := distance
	if focus != null and is_instance_valid(focus):
		look = look.lerp(focus.global_position, 0.42)
		want_dist = distance + 7.0
	_dist = lerpf(_dist, want_dist, 1.0 - exp(-3.0 * real_dt))
	_look = look if _look == Vector3.ZERO else _look.lerp(look, 1.0 - exp(-10.0 * real_dt))

	var offset := Vector3(0, 0, _dist).rotated(Vector3.RIGHT, pitch).rotated(Vector3.UP, yaw)
	global_position = _look + offset
	if global_position.y < 0.6:
		global_position.y = 0.6
	cam.look_at(_look, Vector3.UP)

	_trauma = maxf(0.0, _trauma - real_dt * 1.8)
	var s := _trauma * _trauma
	cam.h_offset = randf_range(-1.0, 1.0) * 0.35 * s
	cam.v_offset = randf_range(-1.0, 1.0) * 0.35 * s
