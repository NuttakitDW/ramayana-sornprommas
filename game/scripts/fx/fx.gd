extends Node3D
## One-shot visual effects: sparks, shockwave rings, damage numbers, ground
## telegraphs, hit-stop and slow motion. Reached via group "fx".

const P := preload("res://scripts/actors/khon_parts.gd")

var _mono: SystemFont
var _hitstop_until_ms := 0
var slowmo_scale := 1.0


func _ready() -> void:
	add_to_group("fx")
	process_mode = Node.PROCESS_MODE_ALWAYS
	_mono = SystemFont.new()
	_mono.font_names = PackedStringArray(["Menlo", "SF Mono", "Courier New", "monospace"])


func _process(_delta: float) -> void:
	if get_tree().paused:
		return
	var stopped := Time.get_ticks_msec() < _hitstop_until_ms
	Engine.time_scale = 0.04 if stopped else slowmo_scale


func hitstop(seconds: float) -> void:
	_hitstop_until_ms = maxi(_hitstop_until_ms, Time.get_ticks_msec() + int(seconds * 1000.0))


func sparks(pos: Vector3, color: Color, amount := 18, speed := 7.0) -> void:
	var p := CPUParticles3D.new()
	var mesh := P.sphere(0.035)
	var m := StandardMaterial3D.new()
	m.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	m.albedo_color = color
	mesh.material = m
	p.mesh = mesh
	p.one_shot = true
	p.amount = amount
	p.lifetime = 0.5
	p.explosiveness = 1.0
	p.direction = Vector3.UP
	p.spread = 180.0
	p.initial_velocity_min = speed * 0.4
	p.initial_velocity_max = speed
	p.gravity = Vector3(0, -14, 0)
	p.scale_amount_min = 0.6
	p.scale_amount_max = 1.6
	add_child(p)
	p.global_position = pos
	p.emitting = true
	_free_later(p, 1.2)


## Expanding flat ring on the ground (landing, shockwave, head break).
func ring(pos: Vector3, radius: float, color: Color, duration := 0.45) -> void:
	var m := StandardMaterial3D.new()
	m.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	m.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
	m.albedo_color = color
	var mi := P.part(self, P.torus(0.85, 1.0), m, pos + Vector3(0, 0.08, 0), Vector3.ZERO, Vector3(0.2, 0.05, 0.2))
	var tw := create_tween().set_parallel(true)
	tw.tween_property(mi, "scale", Vector3(radius, 0.05, radius), duration).set_trans(Tween.TRANS_EXPO).set_ease(Tween.EASE_OUT)
	tw.tween_property(m, "albedo_color:a", 0.0, duration)
	tw.chain().tween_callback(mi.queue_free)


func number(pos: Vector3, value: float, color: Color) -> void:
	text(pos, str(int(round(value))), color, 56)


func text(pos: Vector3, s: String, color: Color, size := 48) -> void:
	var l := Label3D.new()
	l.text = s
	l.font = _mono
	l.font_size = size
	l.pixel_size = 0.0045
	l.outline_size = 10
	l.outline_modulate = Color(0.02, 0.03, 0.06, 0.9)
	l.modulate = color
	l.billboard = BaseMaterial3D.BILLBOARD_ENABLED
	l.no_depth_test = true
	add_child(l)
	l.global_position = pos + Vector3(randf_range(-0.2, 0.2), 0, 0)
	var tw := create_tween().set_parallel(true)
	tw.tween_property(l, "global_position", l.global_position + Vector3(0, 0.9, 0), 0.7).set_ease(Tween.EASE_OUT)
	tw.tween_property(l, "modulate:a", 0.0, 0.7).set_delay(0.25)
	tw.chain().tween_callback(l.queue_free)


## Red warning disc that fills over `duration`; returns the node so callers can
## free it early. Used for Indrajit's arrow volleys.
func telegraph(pos: Vector3, radius: float, duration: float, color: Color) -> Node3D:
	var root := Node3D.new()
	add_child(root)
	root.global_position = Vector3(pos.x, 0.06, pos.z)
	var edge := StandardMaterial3D.new()
	edge.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	edge.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
	edge.albedo_color = Color(color.r, color.g, color.b, 0.9)
	P.part(root, P.torus(radius * 0.94, radius), edge, Vector3.ZERO, Vector3.ZERO, Vector3(1, 0.05, 1))
	var fill := StandardMaterial3D.new()
	fill.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	fill.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
	fill.albedo_color = Color(color.r, color.g, color.b, 0.35)
	var disc := P.part(root, P.cyl(radius, radius, 0.02, 24), fill, Vector3.ZERO, Vector3.ZERO, Vector3(0.05, 1, 0.05))
	var tw := create_tween()
	tw.tween_property(disc, "scale", Vector3.ONE, duration)
	_free_later(root, duration + 0.05)
	return root


func _free_later(node: Node, seconds: float) -> void:
	get_tree().create_timer(seconds, false).timeout.connect(func() -> void:
		if is_instance_valid(node):
			node.queue_free()
	)
