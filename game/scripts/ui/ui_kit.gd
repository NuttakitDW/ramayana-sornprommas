extends RefCounted
## Small UI construction helpers + fonts (macOS system Thai fonts with
## fallbacks). Display: Sathu (traditional looped Thai). Text: Thonburi.
## Numbers / keys / romanization: Menlo (monospace).

const Pal := preload("res://scripts/core/palette.gd")

static var _display: SystemFont
static var _text: SystemFont
static var _mono: SystemFont


static func display_font() -> SystemFont:
	if _display == null:
		_display = SystemFont.new()
		_display.font_names = PackedStringArray(["Sathu", "Ayuthaya", "Thonburi", "Noto Serif Thai", "Noto Sans Thai"])
		_display.fallbacks = [text_font()]
	return _display


static func text_font() -> SystemFont:
	if _text == null:
		_text = SystemFont.new()
		_text.font_names = PackedStringArray(["Thonburi", "Sukhumvit Set", "Noto Sans Thai", "Tahoma"])
	return _text


static func mono_font() -> SystemFont:
	if _mono == null:
		_mono = SystemFont.new()
		_mono.font_names = PackedStringArray(["Menlo", "SF Mono", "Courier New", "monospace"])
		_mono.fallbacks = [text_font()]
	return _mono


static func label(parent: Control, text: String, font: Font, size: int, color: Color, align := HORIZONTAL_ALIGNMENT_LEFT) -> Label:
	var l := Label.new()
	l.text = text
	l.horizontal_alignment = align
	l.add_theme_font_override("font", font)
	l.add_theme_font_size_override("font_size", size)
	l.add_theme_color_override("font_color", color)
	l.add_theme_color_override("font_shadow_color", Color(0, 0, 0, 0.55))
	l.add_theme_constant_override("shadow_offset_x", 0)
	l.add_theme_constant_override("shadow_offset_y", 2)
	l.mouse_filter = Control.MOUSE_FILTER_IGNORE
	parent.add_child(l)
	return l


static func rect(parent: Control, color: Color, pos: Vector2, size: Vector2) -> ColorRect:
	var r := ColorRect.new()
	r.color = color
	r.position = pos
	r.size = size
	r.mouse_filter = Control.MOUSE_FILTER_IGNORE
	parent.add_child(r)
	return r


static func panel_style(alpha := 0.86, border := Pal.UI_BRONZE) -> StyleBoxFlat:
	var s := StyleBoxFlat.new()
	s.bg_color = Color(Pal.UI_PANEL, alpha)
	s.border_color = Color(border, 0.8)
	s.set_border_width_all(1)
	s.border_width_top = 2
	s.content_margin_left = 22
	s.content_margin_right = 22
	s.content_margin_top = 16
	s.content_margin_bottom = 16
	return s


static func vbox(parent: Control, separation := 6) -> VBoxContainer:
	var v := VBoxContainer.new()
	v.add_theme_constant_override("separation", separation)
	v.mouse_filter = Control.MOUSE_FILTER_IGNORE
	parent.add_child(v)
	return v


## A bar made of a background rect and a fill rect; returns the fill.
static func bar(parent: Control, pos: Vector2, size: Vector2, fill_color: Color) -> ColorRect:
	rect(parent, Color(Pal.UI_EDGE, 0.95), pos - Vector2(2, 2), size + Vector2(4, 4))
	rect(parent, Color(Pal.UI_INK, 0.9), pos, size)
	var fill := rect(parent, fill_color, pos, size)
	return fill


## Anchors a control to a point of its parent (0..1 on each axis) and places
## it at `offset` from that point with the given size.
static func place(c: Control, anchor: Vector2, offset: Vector2, size := Vector2.ZERO) -> void:
	c.anchor_left = anchor.x
	c.anchor_right = anchor.x
	c.anchor_top = anchor.y
	c.anchor_bottom = anchor.y
	c.offset_left = offset.x
	c.offset_top = offset.y
	c.offset_right = offset.x + size.x
	c.offset_bottom = offset.y + size.y
