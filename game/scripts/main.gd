extends Node3D
## Entry point: builds the battlefield, Hanuman, camera, music, HUD and the
## battle director. Everything is procedural; no imported assets.
##
## Command-line options (after `--`):
##   --autobot            Hanuman plays himself (for testing)
##   --quick              shorter story beats, auto-confirm cards
##   --start=wave2|boss   jump to a later beat
##   --shots=5,12,30 --shotdir=/abs/path --quit-at=40   screenshot harness

const InputConfig := preload("res://scripts/core/input_config.gd")
const Arena := preload("res://scripts/world/arena_builder.gd")
const Music := preload("res://scripts/audio/music_director.gd")
const Fx := preload("res://scripts/fx/fx.gd")
const Hanuman := preload("res://scripts/actors/hanuman.gd")
const CameraRig := preload("res://scripts/world/camera_rig.gd")
const Hud := preload("res://scripts/ui/hud.gd")
const Director := preload("res://scripts/core/battle_director.gd")
const TestHarness := preload("res://scripts/core/test_harness.gd")


func _ready() -> void:
	InputConfig.register()
	Engine.time_scale = 1.0
	var args := OS.get_cmdline_user_args()
	var autobot := args.has("--autobot")

	Arena.build(self)

	var music := Music.new()
	music.name = "Music"
	add_child(music)

	var fx := Fx.new()
	fx.name = "Fx"
	add_child(fx)

	var player := Hanuman.new()
	player.name = "Hanuman"
	player.autobot = autobot
	add_child(player)
	player.global_position = Vector3(0, 0, 8)

	var cam := CameraRig.new()
	cam.name = "CameraRig"
	cam.target = player
	cam.capture_mouse = not autobot
	add_child(cam)

	var hud := Hud.new()
	hud.name = "Hud"
	hud.player = player
	add_child(hud)
	player.died.connect(hud.hurt_flash)

	var director := Director.new()
	director.name = "Director"
	add_child(director)
	director.begin(player, hud, music, fx, cam, self, args)

	if _has_prefix(args, "--shots=") or _has_prefix(args, "--quit-at="):
		var harness := TestHarness.new()
		add_child(harness)
		harness.configure(args)


func _unhandled_input(event: InputEvent) -> void:
	if event.is_action_pressed("restart"):
		Engine.time_scale = 1.0
		get_tree().reload_current_scene()


static func _has_prefix(args: PackedStringArray, prefix: String) -> bool:
	for a in args:
		if a.begins_with(prefix):
			return true
	return false
