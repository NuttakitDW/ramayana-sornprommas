extends RefCounted
## Rama's side on the southern edge of the field: พระลักษมณ์ (gold, unmasked,
## with bow) and a line of พลวานร (monkey soldiers). They fall when the
## Brahmastra strikes, leaving Hanuman alone, as in the episode.

const Figure := preload("res://scripts/actors/khon_figure.gd")
const Pal := preload("res://scripts/core/palette.gd")

const LAKSHMAN_GOLD := Color(0.84, 0.64, 0.26)
const MONKEY_SKINS := [
	Color(0.62, 0.14, 0.10),
	Color(0.16, 0.45, 0.22),
	Color(0.12, 0.12, 0.15),
	Color(0.15, 0.26, 0.56),
	Color(0.70, 0.52, 0.18),
	Color(0.55, 0.20, 0.30),
]


static func spawn(world: Node3D) -> Node3D:
	var group := Node3D.new()
	group.name = "Allies"
	world.add_child(group)
	group.global_position = Vector3(0, 0, 17.0)
	group.rotation.y = 0.0

	var lakshman := _figure(group, {
		"kind": "phra", "skin": LAKSHMAN_GOLD, "cloth": Pal.KHON_RED,
		"trim": Pal.UI_DEPTH, "crown": "phra", "weapon": "bow", "mouth": "calm",
	}, Vector3(0, 0, 1.0))
	lakshman.set_meta("is_lakshman", true)

	for i in MONKEY_SKINS.size():
		var side := -1.0 if i % 2 == 0 else 1.0
		var slot := float(i / 2 + 1)
		var skin: Color = MONKEY_SKINS[i]
		_figure(group, {
			"kind": "ling", "skin": skin, "cloth": Pal.CLOTH_DARK,
			"trim": Pal.KHON_RED, "crown": "hanuman", "weapon": "club",
			"mouth": "open", "tail": true,
		}, Vector3(side * slot * 1.9, 0, slot * 0.7))
	return group


static func _figure(parent: Node3D, spec: Dictionary, pos: Vector3) -> Node3D:
	var f := Figure.new()
	parent.add_child(f)
	f.build(spec)
	f.position = pos
	return f


static func fall(group: Node3D) -> void:
	var i := 0
	for f in group.get_children():
		var delay := 0.08 * i
		var t := f.get_tree().create_timer(delay, false)
		t.timeout.connect(func() -> void:
			f.flash(Color(1.0, 0.85, 0.5), 0.3)
			f.is_down = true)
		i += 1
