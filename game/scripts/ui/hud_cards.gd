extends Control
## Centre-screen story cards: title card, story beats with verse quotations,
## game-over and end-of-demo cards. Emits `confirmed` when the player presses
## confirm while a card is waiting.

signal confirmed

const Pal := preload("res://scripts/core/palette.gd")
const UI := preload("res://scripts/ui/ui_kit.gd")

const COPYRIGHT := "© 2026 Nuttakit Kundum"

var waiting := false

var _dim: ColorRect
var _panel: PanelContainer
var _kicker: Label
var _title: Label
var _subtitle: Label
var _body: Label
var _source: Label
var _prompt: Label
var _footer: Label
var _blink := 0.0


func _ready() -> void:
	set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	mouse_filter = Control.MOUSE_FILTER_IGNORE
	_dim = UI.rect(self, Color(Pal.UI_INK, 0.55), Vector2.ZERO, Vector2.ZERO)
	_dim.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)

	_panel = PanelContainer.new()
	_panel.add_theme_stylebox_override("panel", UI.panel_style(0.9))
	UI.place(_panel, Vector2(0.5, 0.5), Vector2(-520, -250), Vector2(1040, 0))
	_panel.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(_panel)
	var v := UI.vbox(_panel, 8)
	_kicker = UI.label(v, "", UI.mono_font(), 13, Pal.UI_BRONZE, HORIZONTAL_ALIGNMENT_CENTER)
	_title = UI.label(v, "", UI.display_font(), 60, Pal.UI_PALE, HORIZONTAL_ALIGNMENT_CENTER)
	_subtitle = UI.label(v, "", UI.mono_font(), 15, Pal.UI_SIGNAL, HORIZONTAL_ALIGNMENT_CENTER)
	UI.rect(v, Color(Pal.UI_BRONZE, 0.6), Vector2.ZERO, Vector2(0, 1)).custom_minimum_size = Vector2(0, 1)
	_body = UI.label(v, "", UI.text_font(), 21, Pal.UI_PALE, HORIZONTAL_ALIGNMENT_CENTER)
	_body.autowrap_mode = TextServer.AUTOWRAP_WORD
	_source = UI.label(v, "", UI.mono_font(), 12, Pal.UI_BRONZE, HORIZONTAL_ALIGNMENT_CENTER)
	_prompt = UI.label(v, "", UI.mono_font(), 14, Pal.UI_PALE, HORIZONTAL_ALIGNMENT_CENTER)
	_footer = UI.label(v, "", UI.mono_font(), 11, Pal.UI_ASH, HORIZONTAL_ALIGNMENT_CENTER)
	visible = false


## Keys: kicker, title, subtitle, body (Array of lines), source, prompt, footer.
func show_card(d: Dictionary, wait_for_confirm := false) -> void:
	_kicker.text = d.get("kicker", "")
	_title.text = d.get("title", "")
	_title.visible = _title.text != ""
	_subtitle.text = d.get("subtitle", "")
	_body.text = "\n".join(d.get("body", []))
	_source.text = d.get("source", "")
	_prompt.text = d.get("prompt", "")
	_footer.text = d.get("footer", COPYRIGHT)
	waiting = wait_for_confirm
	visible = true
	modulate.a = 0.0
	create_tween().tween_property(self, "modulate:a", 1.0, 0.35)


func hide_card() -> void:
	waiting = false
	var tw := create_tween()
	tw.tween_property(self, "modulate:a", 0.0, 0.3)
	tw.tween_callback(func() -> void: visible = false)


func _unhandled_input(event: InputEvent) -> void:
	if waiting and visible and event.is_action_pressed("confirm"):
		waiting = false
		get_viewport().set_input_as_handled()
		confirmed.emit()


func _process(delta: float) -> void:
	if not visible or _prompt.text == "":
		return
	_blink += delta / maxf(Engine.time_scale, 0.001)
	_prompt.modulate.a = 0.55 + 0.45 * sin(_blink * 4.0) if waiting else 1.0
