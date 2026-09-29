extends Node
## Runs the demo's story beats (after lore/story/02-synopsis.md):
##   title → march (กราวนอก) → vanguard wave (เชิด) → Kampan's wave (เชิดกลอง)
##   → Erawan appears, the Brahmastra fells the army (เชิดฉิ่ง / ร่ายรุด)
##   → Hanuman rages (ลิงโลด) → boss → neck broken, bow seized, Hanuman struck
##   down (ลิงลาน / โอดแหบ) → revived by the wind (แขกบรเทศ) → end card.
## Per Khon convention the show never ends on the fallen tableau (พระล้ม).

const Yaksha := preload("res://scripts/actors/yaksha.gd")
const Boss := preload("res://scripts/actors/erawan_boss.gd")
const Allies := preload("res://scripts/world/allies.gd")
const Pal := preload("res://scripts/core/palette.gd")
const Story := preload("res://scripts/core/story_text.gd")
const UI := preload("res://scripts/ui/ui_kit.gd")

var player: Node
var hud: Node
var music: Node
var fx: Node
var cam: Node
var world: Node3D

var phase := ""
var quick := false
var auto_confirm := false
var max_tokens := 2

var _tokens: Array = []
var _alive := 0
var _wave_total := 0
var _wave_killed := 0
var _objective: Array = ["", ""]
var _over := false
var _boss: Node3D
var _allies: Node3D
var _start_ms := 0


func _ready() -> void:
	add_to_group("director")


func begin(p: Node, h: Node, m: Node, f: Node, c: Node, w: Node3D, args: PackedStringArray) -> void:
	player = p
	hud = h
	music = m
	fx = f
	cam = c
	world = w
	quick = args.has("--quick")
	auto_confirm = quick or args.has("--autobot")
	player.died.connect(_on_player_died)
	_allies = Allies.spawn(world)
	var start := "intro"
	for a in args:
		if a.begins_with("--start="):
			start = a.substr(8)
	_start_ms = Time.get_ticks_msec()
	_run(start)


func _run(start: String) -> void:
	var order := ["intro", "wave1", "wave2", "boss"]
	var from := maxi(order.find(start), 0)
	if from <= 0:
		await _intro()
	if from <= 1 and not _over:
		await _wave1()
	if from <= 2 and not _over:
		await _wave2()
	if not _over:
		await _boss_intro()
	if not _over:
		await _boss_fight()
	if not _over:
		await _finale()


# ---------------------------------------------------------------- beats

func _intro() -> void:
	phase = "intro"
	player.input_enabled = false
	music.play_cue("wa")
	hud.cards.show_card(Story.TITLE, true)
	await _confirm()
	hud.cards.hide_card()
	music.play_cue("kraw_nok")
	hud.banner(Story.MARCH["lines"], Story.MARCH["source"], 3.5)
	await _wait(3.0)


func _wave1() -> void:
	phase = "wave1"
	_start_ms = Time.get_ticks_msec()
	player.input_enabled = true
	music.play_cue("choet")
	_set_objective("ปราบพลยักษ์ขัดตาทัพ", "DEFEAT THE DEMON VANGUARD")
	_start_wave(4)
	for i in 4:
		_spawn_soldier(false)
	await _wave_cleared()


func _wave2() -> void:
	phase = "wave2"
	player.input_enabled = true
	max_tokens = 3
	music.play_cue("choet_klong")
	hud.banner(Story.KAMPAN["lines"], Story.KAMPAN["source"], 6.0)
	_set_objective("ปราบกำปั่นและทัพยักษ์", "DEFEAT KAMPAN AND HIS DEMONS")
	_start_wave(5)
	var captain := _spawn_soldier(true)
	_name_tag(captain, "กำปั่น", "KAMPAN")
	for i in 4:
		_spawn_soldier(false)
	await _wave_cleared()


func _boss_intro() -> void:
	phase = "boss_intro"
	player.enter_cutscene()
	_set_objective("", "")
	await _wait(0.8)
	_boss = Boss.new()
	world.add_child(_boss)
	_boss.appear()
	cam.focus = _boss
	music.play_cue("choet_ching")
	hud.banner(Story.ERAWAN_APPEARS["lines"], Story.ERAWAN_APPEARS["source"], 4.0)
	await _wait(3.6)
	hud.banner(Story.ARROW_LOOSED["lines"], Story.ARROW_LOOSED["source"], 4.0)
	_boss.model.rider.play("bow_draw", 1.2, true)
	await _wait(2.4)
	_loose_brahmastra()
	hud.banner(Story.ARMY_FALLS["lines"], Story.ARMY_FALLS["source"], 4.5)
	await _wait(4.0)
	music.play_cue("ling_lot")
	player.face_point(_boss.global_position)
	hud.banner(Story.LING_LOT["lines"], Story.LING_LOT["source"], 6.0)
	await _wait(3.5)
	player.leave_cutscene()


func _boss_fight() -> void:
	phase = "boss"
	max_tokens = 2
	hud.show_boss(_boss)
	_boss.active = true
	music.play_cue("choet_klong")
	_boss_objective()
	_boss.head_broken.connect(_on_head_broken)
	await _boss.defeated


func _finale() -> void:
	phase = "finale"
	player.enter_cutscene()
	fx.slowmo_scale = 0.35
	_kill_all_soldiers()
	hud.hide_boss()
	_set_objective("", "")
	hud.banner(Story.NECK_BREAK["lines"], Story.NECK_BREAK["source"], 5.0)
	await _wait(1.4)
	fx.slowmo_scale = 1.0
	await _leap_to_rider()
	# Indrajit swings the bow and strikes Hanuman down.
	_boss.model.rider.play("bow_release", 0.4)
	music.play_cue("ot_haep")
	hud.hurt_flash()
	cam.shake(0.8)
	fx.sparks(player.global_position + Vector3(0, 1.5, 0), Color.WHITE, 40, 9.0)
	hud.banner(Story.STRUCK_DOWN["lines"], Story.STRUCK_DOWN["source"], 6.0)
	await _fall_to_ground()
	_boss_vanish()
	await _wait(3.5)
	hud.fade_to(0.72, 1.5)
	await _wait(2.0)
	hud.cards.show_card(Story.WIND, false)
	_wind_gust()
	await _wait(4.5)
	hud.cards.hide_card()
	hud.fade_to(0.0, 1.2)
	player.revive()
	music.play_cue("khaek_bora_thet")
	await _wait(1.2)
	player.model.play("victory", 3.0, true)
	await _wait(2.5)
	phase = "end"
	hud.cards.show_card(Story.end_card(_stats()), true)


# ---------------------------------------------------------------- set pieces

func _loose_brahmastra() -> void:
	var from: Vector3 = _boss.model.rider.global_position + Vector3(0, 1.4, 0)
	var to: Vector3 = _allies.global_position + Vector3(0, 1.0, 0)
	var m := StandardMaterial3D.new()
	m.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	m.albedo_color = Color(1.0, 0.85, 0.5)
	m.emission_enabled = true
	m.emission = Color(1.0, 0.8, 0.4)
	m.emission_energy_multiplier = 6.0
	var beam := MeshInstance3D.new()
	var mesh := CylinderMesh.new()
	mesh.top_radius = 0.12
	mesh.bottom_radius = 0.12
	mesh.height = from.distance_to(to)
	beam.mesh = mesh
	beam.material_override = m
	world.add_child(beam)
	beam.global_position = (from + to) * 0.5
	beam.look_at(to, Vector3.UP)
	beam.rotate_object_local(Vector3.RIGHT, PI / 2.0)
	var tw := create_tween()
	tw.tween_property(beam, "scale", Vector3(0.1, 1.0, 0.1), 0.6)
	tw.tween_callback(beam.queue_free)
	fx.ring(Vector3(to.x, 0, to.z), 14.0, Color(1.0, 0.8, 0.4, 0.9), 0.9)
	fx.sparks(to, Color(1.0, 0.85, 0.5), 60, 12.0)
	cam.shake(0.7)
	music.sfx("klong", 0.4, 4.0)
	Allies.fall(_allies)


func _leap_to_rider() -> void:
	player.set_physics_process(false)
	var top: Vector3 = _boss.model.rider.global_position + Vector3(0, 0.2, 1.1)
	var start: Vector3 = player.global_position
	var mid := (start + top) * 0.5 + Vector3(0, 4.0, 0)
	player.face_point(_boss.global_position)
	player.model.grounded = false
	var tw := create_tween()
	tw.tween_method(func(t: float) -> void:
		player.global_position = start.lerp(mid, t).lerp(mid.lerp(top, t), t), 0.0, 1.0, 0.8)
	await tw.finished
	player.model.play("slam", 0.5)
	await _wait(0.5)


func _fall_to_ground() -> void:
	var start: Vector3 = player.global_position
	var land := Vector3(start.x * 0.6, 0.0, start.z * 0.6 + 3.0)
	var tw := create_tween()
	tw.tween_property(player, "global_position", land, 0.9).set_trans(Tween.TRANS_QUAD).set_ease(Tween.EASE_IN)
	await tw.finished
	player.knock_down()
	player.model.grounded = true
	fx.ring(land, 4.0, Color(Pal.GOLD, 0.7))
	cam.shake(0.6)
	music.sfx("klong", 0.6, 3.0)
	player.set_physics_process(true)


func _boss_vanish() -> void:
	cam.focus = null
	var tw := create_tween().set_parallel(true)
	tw.tween_property(_boss, "global_position", _boss.global_position + Vector3(0, 30, -30), 3.0).set_trans(Tween.TRANS_SINE)
	tw.tween_property(_boss, "scale", Vector3.ONE * 0.2, 3.0)
	tw.chain().tween_callback(_boss.queue_free)


func _wind_gust() -> void:
	for i in 6:
		var t := get_tree().create_timer(i * 0.5, false, false, true)
		t.timeout.connect(func() -> void:
			fx.sparks(player.global_position + Vector3(randf_range(-1, 1), 0.5, randf_range(-1, 1)), Color(0.75, 0.88, 1.0), 24, 5.0)
			fx.ring(player.global_position, 3.5, Color(0.75, 0.88, 1.0, 0.6), 0.8))


# ---------------------------------------------------------------- waves

func _start_wave(total: int) -> void:
	_wave_total = total
	_wave_killed = 0
	_update_wave_objective()


func _spawn_soldier(captain: bool) -> Node3D:
	var y := Yaksha.new()
	var skins: Array = Pal.YAKSHA_SKINS
	y.configure(Pal.KHON_RED if captain else skins[randi() % skins.size()], captain)
	var a := randf_range(-PI * 0.85, -PI * 0.15)
	world.add_child(y)
	y.global_position = Vector3(cos(a) * 23.0, 0.0, sin(a) * 23.0)
	y.died.connect(_on_enemy_died)
	_alive += 1
	return y


func _name_tag(node: Node3D, th: String, en: String) -> void:
	var l := Label3D.new()
	l.text = "%s\n%s" % [th, en]
	l.font = UI.text_font()
	l.font_size = 64
	l.pixel_size = 0.004
	l.outline_size = 12
	l.modulate = Pal.GOLD
	l.billboard = BaseMaterial3D.BILLBOARD_ENABLED
	l.position = Vector3(0, 3.6, 0)
	node.add_child(l)


func _on_enemy_died(_enemy: Node) -> void:
	_alive -= 1
	_wave_killed += 1
	_update_wave_objective()


func _update_wave_objective() -> void:
	if _wave_total > 0 and phase.begins_with("wave"):
		hud.set_objective(_objective[0], "%s   %d/%d" % [_objective[1], _wave_killed, _wave_total])


func _wave_cleared() -> void:
	while _alive > 0 and not _over:
		await get_tree().create_timer(0.25, false).timeout
	await _wait(0.8)


func _kill_all_soldiers() -> void:
	for e in get_tree().get_nodes_in_group("enemies"):
		if e.is_alive():
			e.receive_hit(9999.0, player.global_position, 6.0, 0)


func _on_head_broken(_index: int, remaining: int) -> void:
	_boss_objective()
	if remaining == 2:
		hud.banner(Story.NECK_BREAK["lines"], Story.NECK_BREAK["source"], 4.0)
	if remaining > 0:
		for i in 2:
			_spawn_soldier(false)


func _boss_objective() -> void:
	var broken: int = 3 - _boss.heads_left()
	hud.set_objective("หักคอเอราวัณ  ฟาดเศียรช้างขณะคุกเข่า", "BREAK ERAWAN'S NECKS: STRIKE THE HEADS WHILE IT KNEELS   %d/3" % broken)


func _set_objective(th: String, en: String) -> void:
	_objective = [th, en]
	hud.set_objective(th, en)


# ---------------------------------------------------------------- tokens

func request_token(enemy: Node) -> bool:
	_tokens = _tokens.filter(func(e: Node) -> bool: return is_instance_valid(e) and e.is_alive())
	if enemy in _tokens:
		return true
	if _tokens.size() < max_tokens:
		_tokens.append(enemy)
		return true
	return false


func release_token(enemy: Node) -> void:
	_tokens.erase(enemy)


# ---------------------------------------------------------------- flow helpers

func _on_player_died() -> void:
	if _over:
		return
	_over = true
	phase = "game_over"
	if _boss != null and is_instance_valid(_boss):
		_boss.active = false
	music.play_cue("kraw_ram_phama")
	hud.hide_boss()
	await _wait(1.5)
	hud.cards.show_card(Story.GAME_OVER, false)


func _wait(seconds: float) -> void:
	var s := seconds * (0.4 if quick else 1.0)
	await get_tree().create_timer(s, false, false, true).timeout


func _confirm() -> void:
	if auto_confirm:
		await _wait(1.5)
	else:
		await hud.cards.confirmed


func _stats() -> String:
	var secs := int((Time.get_ticks_msec() - _start_ms) / 1000.0)
	return "TIME %02d:%02d    MAX COMBO %d    HITS TAKEN %d" % [secs / 60, secs % 60, player.max_combo, player.hits_taken]
