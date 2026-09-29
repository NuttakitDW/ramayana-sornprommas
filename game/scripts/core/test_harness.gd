extends Node
## Automated play-test helper: saves screenshots at given real-time seconds,
## prints a state line every few seconds, and quits at --quit-at.

var _shots: Array[float] = []
var _shot_dir := "user://shots"
var _quit_at := -1.0
var _elapsed := 0.0
var _next_log := 0.0
var _taken := 0


func _ready() -> void:
	process_mode = Node.PROCESS_MODE_ALWAYS


func configure(args: PackedStringArray) -> void:
	for a in args:
		if a.begins_with("--shots="):
			for s in a.substr(8).split(",", false):
				_shots.append(float(s))
		elif a.begins_with("--shotdir="):
			_shot_dir = a.substr(10)
		elif a.begins_with("--quit-at="):
			_quit_at = float(a.substr(10))
	DirAccess.make_dir_recursive_absolute(_shot_dir)


func _process(delta: float) -> void:
	_elapsed += delta / maxf(Engine.time_scale, 0.001)
	if _elapsed >= _next_log:
		_next_log += 3.0
		_log_state()
	if _taken < _shots.size() and _elapsed >= _shots[_taken]:
		var t := _shots[_taken]
		_taken += 1
		_capture(t)
	if _quit_at > 0.0 and _elapsed >= _quit_at:
		print("[harness] quit at %.1fs" % _elapsed)
		get_tree().quit()


func _capture(t: float) -> void:
	await RenderingServer.frame_post_draw
	var img := get_viewport().get_texture().get_image()
	var path := "%s/shot_%02d_%03ds.png" % [_shot_dir, _taken, int(t)]
	var err := img.save_png(path)
	print("[harness] screenshot %s (%s)" % [path, error_string(err)])


func _log_state() -> void:
	var p := get_tree().get_first_node_in_group("player")
	var d := get_tree().get_first_node_in_group("director")
	var b := get_tree().get_first_node_in_group("boss")
	var m := get_tree().get_first_node_in_group("music")
	var boss_txt := "-"
	if b != null and is_instance_valid(b):
		boss_txt = "%s heads=%s" % [b.state, str(b.head_hp)]
	print("[harness] t=%.1f phase=%s hp=%.0f power=%.0f state=%s enemies=%d boss=%s cue=%s" % [
		_elapsed, d.phase if d else "?", p.hp if p else -1.0, p.power if p else -1.0,
		p.state if p else "?", get_tree().get_nodes_in_group("enemies").size(), boss_txt,
		m.current if m else "?"])
