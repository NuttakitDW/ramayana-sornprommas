extends RefCounted
## Procedural pose library for khon_figure.gd. Returns target rotations (radians)
## per pivot. Conventions: +X on a limb swings it forward; +X on the chest leans
## it back; +Y turns left; +Z moves a hanging limb toward +X (the figure's right).

const STANCE := {
	"ling": {"crouch": 0.14, "open": 0.4},
	"yak": {"crouch": 0.1, "open": 0.44},
	"phra": {"crouch": 0.05, "open": 0.2},
}


static func compute(c: Dictionary) -> Dictionary:
	var kind: String = c["kind"]
	var p := _stance(kind, c["beat"], c["time"])
	if not c["grounded"]:
		_air(p)
	elif c["move"] > 0.05:
		_run(p, kind, c["move"], c["run_phase"])
	if c["action"] != "":
		_action(p, c["action"], c["u"])
	return p


## Khon standing posture: knees open and bent, chest lifted, with the ยืดยุบ sink on each beat.
static func _stance(kind: String, beat: float, t: float) -> Dictionary:
	var s: Dictionary = STANCE.get(kind, STANCE["ling"])
	var sink := pow(sin(PI * beat), 2.0)
	var crouch: float = s["crouch"]
	var open: float = s["open"]
	var p := {
		"hips_offset": Vector3(0, -crouch - 0.035 * sink, 0),
		"leg_l": Vector3(0.35, 0, -open),
		"leg_r": Vector3(0.35, 0, open),
		"shin_l": Vector3(-0.72 - 0.12 * sink, 0, 0),
		"shin_r": Vector3(-0.72 - 0.12 * sink, 0, 0),
		"chest": Vector3(0.08, 0, 0),
		"head": Vector3(-0.05, 0, 0),
		"arm_l": Vector3(0.3, 0, -0.95),
		"fore_l": Vector3(0.9, 0, -0.9),
		"arm_r": Vector3(0.45, 0, 0.35),
		"fore_r": Vector3(1.15, 0, 0),
		"weapon": Vector3(-PI / 2.0, 0, 0),
	}
	if kind == "ling":
		p["head"] = Vector3(-0.05, sin(t * 2.7) * 0.22, sin(t * 1.3) * 0.08)
	elif kind == "phra":
		p["arm_l"] = Vector3(1.1, 0, -0.25)
		p["fore_l"] = Vector3(0.35, 0, 0)
	return p


static func _run(p: Dictionary, kind: String, mv: float, ph: float) -> void:
	var s := sin(ph)
	var c := cos(ph)
	var amp := 0.95 * mv
	p["leg_l"] = Vector3(0.25 + s * amp, 0, -0.14)
	p["leg_r"] = Vector3(0.25 - s * amp, 0, 0.14)
	p["shin_l"] = Vector3(-0.45 - maxf(0.0, -c) * 1.2 * mv, 0, 0)
	p["shin_r"] = Vector3(-0.45 - maxf(0.0, c) * 1.2 * mv, 0, 0)
	p["chest"] = Vector3(-0.3 * mv, 0, 0)
	p["arm_l"] = Vector3(0.2 - s * 0.8 * mv, 0, -0.35)
	p["fore_l"] = Vector3(0.9, 0, 0)
	p["arm_r"] = Vector3(0.5 + s * 0.35 * mv, 0, 0.25)
	var hop := 2.0 if kind == "ling" else 1.0
	p["hips_offset"] = Vector3(0, -0.08 + absf(s) * 0.06 * hop, 0)


## ท่าเหาะ / leap: one knee drawn up, free arm flung out.
static func _air(p: Dictionary) -> void:
	p["leg_l"] = Vector3(1.3, 0, -0.2)
	p["shin_l"] = Vector3(-1.7, 0, 0)
	p["leg_r"] = Vector3(0.1, 0, 0.15)
	p["shin_r"] = Vector3(-0.5, 0, 0)
	p["arm_l"] = Vector3(0.4, 0, -1.6)
	p["fore_l"] = Vector3(0.8, 0, 0)
	p["chest"] = Vector3(-0.15, 0, 0)


static func _ease_out(x: float) -> float:
	var v := clampf(x, 0.0, 1.0)
	return 1.0 - pow(1.0 - v, 3.0)


static func _phase(u: float, split: float, a: float, b: float, c: float, strike_len := 0.3) -> float:
	if u < split:
		return lerpf(a, b, u / split)
	return lerpf(b, c, _ease_out((u - split) / strike_len))


static func _action(p: Dictionary, name: String, u: float) -> void:
	match name:
		"swing_a":
			p["chest"] = Vector3(-0.15, _phase(u, 0.3, 0.0, -0.9, 1.05), 0)
			p["arm_r"] = Vector3(1.35, 0, 0.35)
			p["fore_r"] = Vector3(0.1, 0, 0)
			p["weapon"] = Vector3(-PI, 0, 0)
		"swing_b":
			p["chest"] = Vector3(-0.15, _phase(u, 0.3, 0.0, 0.9, -1.05), 0)
			p["arm_r"] = Vector3(1.35, 0, -0.1)
			p["fore_r"] = Vector3(0.1, 0, 0)
			p["weapon"] = Vector3(-PI, 0, 0)
		"slam":
			p["arm_r"] = Vector3(_phase(u, 0.35, 1.5, 3.0, 0.9, 0.25), 0, 0.15)
			p["fore_r"] = Vector3(0.05, 0, 0)
			p["chest"] = Vector3(0.3 if u < 0.35 else -0.5, 0, 0)
			p["weapon"] = Vector3(-PI, 0, 0)
			if u >= 0.5:
				p["hips_offset"] = p["hips_offset"] + Vector3(0, -0.1, 0)
		"thrust":
			p["weapon"] = Vector3(-PI, 0, 0)
			if u < 0.4:
				p["arm_r"] = Vector3(0.8, 0, 0.2)
				p["fore_r"] = Vector3(0.2, 0, 0)
				p["chest"] = Vector3(0.1, -0.6, 0)
			else:
				p["arm_r"] = Vector3(1.57, 0, 0.05)
				p["fore_r"] = Vector3(0.0, 0, 0)
				p["chest"] = Vector3(-0.35, 0.15, 0)
				p["leg_l"] = Vector3(0.95, 0, -0.2)
				p["shin_l"] = Vector3(-0.45, 0, 0)
				p["leg_r"] = Vector3(-0.4, 0, 0.2)
		"plunge":
			p["arm_r"] = Vector3(0.35, 0, 0.1)
			p["fore_r"] = Vector3(0.0, 0, 0)
			p["weapon"] = Vector3(-PI, 0, 0)
			p["chest"] = Vector3(-0.35, 0, 0)
		"special":
			p["arm_l"] = Vector3(0.2, 0, -2.5)
			p["arm_r"] = Vector3(0.2, 0, 2.5)
			p["fore_l"] = Vector3(0.5, 0, 0)
			p["fore_r"] = Vector3(0.5, 0, 0)
			p["chest"] = Vector3(0.3, 0, 0)
			p["head"] = Vector3(0.35, 0, 0)
		"hurt":
			p["chest"] = Vector3(0.45, 0, 0)
			p["head"] = Vector3(0.3, 0, 0)
			p["arm_l"] = Vector3(0.6, 0, -1.3)
		"windup":
			p["arm_r"] = Vector3(2.9, 0, 0.3)
			p["fore_r"] = Vector3(0.4, 0, 0)
			p["weapon"] = Vector3(-PI * 0.9, 0, 0)
			p["chest"] = Vector3(0.25, -0.3, 0)
		"strike":
			p["arm_r"] = Vector3(_phase(u, 0.05, 2.9, 2.9, 0.7, 0.4), 0, 0.2)
			p["fore_r"] = Vector3(0.1, 0, 0)
			p["weapon"] = Vector3(-PI, 0, 0)
			p["chest"] = Vector3(_phase(u, 0.05, 0.25, 0.25, -0.45, 0.4), _phase(u, 0.05, -0.3, -0.3, 0.3, 0.4), 0)
		"bow_draw":
			p["arm_l"] = Vector3(1.5, 0, -0.1)
			p["fore_l"] = Vector3(0.0, 0, 0)
			p["arm_r"] = Vector3(1.45, 0, 0.3)
			p["fore_r"] = Vector3(1.8 * clampf(u * 1.5, 0.0, 1.0), 0, 0)
			p["chest"] = Vector3(0.05, -0.3, 0)
		"bow_release":
			p["arm_l"] = Vector3(1.5, 0, -0.1)
			p["arm_r"] = Vector3(1.2, 0, 0.9)
			p["fore_r"] = Vector3(0.2, 0, 0)
		"victory":
			p["arm_l"] = Vector3(0.3, 0, -2.3)
			p["arm_r"] = Vector3(0.6, 0, 1.2)
			p["fore_l"] = Vector3(0.9, 0, 0)
			p["chest"] = Vector3(0.2, 0.25, 0)
