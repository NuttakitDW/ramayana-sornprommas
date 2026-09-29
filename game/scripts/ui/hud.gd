extends CanvasLayer
## In-game HUD: Hanuman's vitals, objective, the ปี่พาทย์ cue panel with a
## ฉิ่ง/ฉับ beat light, combo counter, boss bar, verse banner and controls.
## Story cards live in hud_cards.gd.

const Pal := preload("res://scripts/core/palette.gd")
const UI := preload("res://scripts/ui/ui_kit.gd")
const Cards := preload("res://scripts/ui/hud_cards.gd")

const BAR_W := 380.0

var player: Node
var boss: Node
var cards: Control

var _root: Control
var _hp_fill: ColorRect
var _hp_text: Label
var _power_fill: ColorRect
var _power_text: Label
var _objective: Label
var _objective_en: Label
var _cue_th: Label
var _cue_rom: Label
var _cue_note: Label
var _ching: Label
var _chap: Label
var _combo: Label
var _combo_sub: Label
var _boss_box: Control
var _boss_fills: Array[ColorRect] = []
var _banner: PanelContainer
var _banner_text: Label
var _banner_src: Label
var _help: Label
var _vignette: ColorRect
var _fade: ColorRect
var _banner_left := 0.0


func _ready() -> void:
	add_to_group("hud")
	process_mode = Node.PROCESS_MODE_ALWAYS
	_root = Control.new()
	_root.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	_root.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(_root)
	_vignette = UI.rect(_root, Color(Pal.DANGER, 0.0), Vector2.ZERO, Vector2.ZERO)
	_vignette.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	_build_player_panel()
	_build_objective()
	_build_cue_panel()
	_build_combo()
	_build_boss_bar()
	_build_banner()
	_build_help()
	_fade = UI.rect(_root, Color(0, 0, 0, 0), Vector2.ZERO, Vector2.ZERO)
	_fade.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	cards = Cards.new()
	_root.add_child(cards)
	var music := get_tree().get_first_node_in_group("music")
	if music:
		music.beat.connect(_on_beat)


func _build_player_panel() -> void:
	var x := 36.0
	UI.label(_root, "หนุมาน", UI.display_font(), 44, Pal.UI_PALE).position = Vector2(x, 14)
	UI.label(_root, "HANUMAN / วายุบุตร  SON OF THE WIND", UI.mono_font(), 13, Pal.UI_BRONZE).position = Vector2(x + 2, 76)
	_hp_fill = UI.bar(_root, Vector2(x, 102), Vector2(BAR_W, 14), Pal.UI_HP)
	_hp_text = UI.label(_root, "", UI.mono_font(), 13, Pal.UI_PALE)
	_hp_text.position = Vector2(x + BAR_W + 12, 99)
	_power_fill = UI.bar(_root, Vector2(x, 126), Vector2(BAR_W, 8), Pal.UI_SIGNAL)
	_power_text = UI.label(_root, "", UI.mono_font(), 13, Pal.UI_ASH)
	_power_text.position = Vector2(x, 140)


func _build_objective() -> void:
	var box := UI.vbox(_root, 0)
	UI.place(box, Vector2(0.5, 0), Vector2(-400, 22), Vector2(800, 0))
	_objective = UI.label(box, "", UI.text_font(), 24, Pal.UI_PALE, HORIZONTAL_ALIGNMENT_CENTER)
	_objective_en = UI.label(box, "", UI.mono_font(), 13, Pal.UI_BRONZE, HORIZONTAL_ALIGNMENT_CENTER)


func _build_cue_panel() -> void:
	var panel := PanelContainer.new()
	panel.add_theme_stylebox_override("panel", UI.panel_style(0.72, Pal.UI_EDGE))
	UI.place(panel, Vector2(1, 0), Vector2(-340, 22), Vector2(310, 0))
	panel.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_root.add_child(panel)
	var v := UI.vbox(panel, 2)
	UI.label(v, "PI PHAT CUE / เพลงปี่พาทย์", UI.mono_font(), 11, Pal.UI_BRONZE)
	_cue_th = UI.label(v, "-", UI.display_font(), 30, Pal.UI_PALE)
	_cue_rom = UI.label(v, "", UI.mono_font(), 12, Pal.UI_SIGNAL)
	var beats := HBoxContainer.new()
	beats.add_theme_constant_override("separation", 14)
	v.add_child(beats)
	_ching = UI.label(beats, "ฉิ่ง", UI.text_font(), 16, Pal.UI_ASH)
	_chap = UI.label(beats, "ฉับ", UI.text_font(), 16, Pal.UI_ASH)
	_cue_note = UI.label(v, "", UI.mono_font(), 10, Pal.UI_ASH)
	_cue_note.autowrap_mode = TextServer.AUTOWRAP_WORD
	_cue_note.custom_minimum_size = Vector2(266, 0)


func _build_combo() -> void:
	_combo = UI.label(_root, "", UI.mono_font(), 54, Pal.UI_PALE, HORIZONTAL_ALIGNMENT_RIGHT)
	UI.place(_combo, Vector2(1, 0.5), Vector2(-260, -60), Vector2(220, 0))
	_combo_sub = UI.label(_root, "", UI.mono_font(), 13, Pal.UI_BRONZE, HORIZONTAL_ALIGNMENT_RIGHT)
	UI.place(_combo_sub, Vector2(1, 0.5), Vector2(-260, 6), Vector2(220, 0))


func _build_boss_bar() -> void:
	_boss_box = Control.new()
	UI.place(_boss_box, Vector2(0.5, 1), Vector2(-390, -112), Vector2(780, 90))
	_boss_box.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_root.add_child(_boss_box)
	UI.label(_boss_box, "อินทรชิตแปลงเป็นพระอินทร์ ทรงช้างเอราวัณ", UI.text_font(), 22, Pal.UI_PALE).position = Vector2(0, 0)
	UI.label(_boss_box, "INDRAJIT DISGUISED AS INDRA, ON ERAWAN (THE DEMON KARUNARAT TRANSFORMED)", UI.mono_font(), 11, Pal.UI_BRONZE).position = Vector2(0, 34)
	for i in 3:
		_boss_fills.append(UI.bar(_boss_box, Vector2(i * 264.0, 58), Vector2(252, 12), Pal.GOLD))
		UI.label(_boss_box, "เศียร %d / HEAD %d" % [i + 1, i + 1], UI.mono_font(), 11, Pal.UI_ASH).position = Vector2(i * 264.0, 74)
	_boss_box.visible = false


func _build_banner() -> void:
	_banner = PanelContainer.new()
	_banner.add_theme_stylebox_override("panel", UI.panel_style(0.8))
	UI.place(_banner, Vector2(0.5, 1), Vector2(-460, -290), Vector2(920, 0))
	_banner.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_root.add_child(_banner)
	var v := UI.vbox(_banner, 4)
	_banner_text = UI.label(v, "", UI.text_font(), 22, Pal.UI_PALE, HORIZONTAL_ALIGNMENT_CENTER)
	_banner_src = UI.label(v, "", UI.mono_font(), 11, Pal.UI_BRONZE, HORIZONTAL_ALIGNMENT_CENTER)
	_banner.modulate.a = 0.0


func _build_help() -> void:
	_help = UI.label(_root, "", UI.mono_font(), 12, Pal.UI_ASH)
	UI.place(_help, Vector2(0, 1), Vector2(36, -118), Vector2(560, 0))
	_help.text = "\n".join([
		"WASD      move          MOUSE / ARROWS  camera",
		"J / LMB   trident combo  K / RMB         heavy thrust",
		"SPACE     leap           J in air        plunge",
		"SHIFT     dash           Q               นิมิตกาย (grow)",
		"R         restart        H               hide help",
	])


# ---------------------------------------------------------------- API

func set_objective(th: String, en: String) -> void:
	_objective.text = th
	_objective_en.text = en


func banner(lines: Array, source: String, seconds: float) -> void:
	_banner_text.text = "\n".join(lines)
	_banner_src.text = source
	_banner_left = seconds
	create_tween().tween_property(_banner, "modulate:a", 1.0, 0.35)


func show_boss(b: Node) -> void:
	boss = b
	_boss_box.visible = true
	_help.visible = false


func hide_boss() -> void:
	_boss_box.visible = false


func fade_to(alpha: float, seconds: float) -> Tween:
	var tw := create_tween()
	tw.tween_property(_fade, "color:a", alpha, seconds)
	return tw


func hurt_flash() -> void:
	_vignette.color.a = 0.28
	create_tween().tween_property(_vignette, "color:a", 0.0, 0.35)


# ---------------------------------------------------------------- update

func _unhandled_input(event: InputEvent) -> void:
	if event.is_action_pressed("toggle_help"):
		_help.visible = not _help.visible


func _process(delta: float) -> void:
	var real_dt := delta / maxf(Engine.time_scale, 0.001)
	_update_player()
	_update_cue()
	_update_boss()
	if _banner_left > 0.0:
		_banner_left -= real_dt
		if _banner_left <= 0.0:
			create_tween().tween_property(_banner, "modulate:a", 0.0, 0.6)


func _update_player() -> void:
	if player == null:
		return
	var hp_ratio: float = player.hp / player.MAX_HP
	_hp_fill.size.x = BAR_W * hp_ratio
	_hp_text.text = "HP %3d/%d" % [int(player.hp), int(player.MAX_HP)]
	_power_fill.size.x = BAR_W * player.power / 100.0
	if player.giant_left > 0.0:
		_power_text.text = "นิมิตกาย  NIMIT KAI  %.1fs" % player.giant_left
		_power_text.add_theme_color_override("font_color", Pal.UI_SIGNAL)
	elif player.power >= 100.0:
		var pulse := 0.6 + 0.4 * sin(Time.get_ticks_msec() * 0.008)
		_power_text.text = "นิมิตกาย  NIMIT KAI  READY  [Q]"
		_power_text.add_theme_color_override("font_color", Color(Pal.UI_SIGNAL, pulse))
	else:
		_power_text.text = "นิมิตกาย  NIMIT KAI  %3d%%" % int(player.power)
		_power_text.add_theme_color_override("font_color", Pal.UI_ASH)
	if player.combo_count >= 2:
		_combo.text = str(player.combo_count)
		_combo_sub.text = "HITS / ครั้ง"
	else:
		_combo.text = ""
		_combo_sub.text = ""


func _update_cue() -> void:
	var music := get_tree().get_first_node_in_group("music")
	if music == null or music.current == "":
		return
	var info: Dictionary = music.cue_info()
	_cue_th.text = info.get("th", "")
	_cue_rom.text = "%s   %d BPM" % [info.get("rom", ""), info.get("bpm", 0)]
	if music.using_recording:
		_cue_note.text = info.get("use", "")
	else:
		_cue_note.text = "%s\nplaceholder percussion, not the real piece" % info.get("use", "")


func _update_boss() -> void:
	if boss == null or not is_instance_valid(boss) or not _boss_box.visible:
		return
	for i in 3:
		_boss_fills[i].size.x = 252.0 * boss.head_hp[i] / boss.HEAD_HP


func _on_beat(_index: int, is_chap: bool) -> void:
	var on := _chap if is_chap else _ching
	var off := _ching if is_chap else _chap
	on.add_theme_color_override("font_color", Pal.UI_SIGNAL if is_chap else Pal.UI_BRONZE)
	off.add_theme_color_override("font_color", Color(Pal.UI_ASH, 0.5))
