extends RefCounted
## Builds the battlefield before Lanka (สนามรบหน้ากรุงลงกา) at dusk:
## sky, light, ground, a gilded stage ring, battle standards, torches and
## the silhouette of Lanka on the northern horizon.

const P := preload("res://scripts/actors/khon_parts.gd")
const Pal := preload("res://scripts/core/palette.gd")

const ARENA_RADIUS := 20.0


static func build(root: Node3D) -> void:
	_environment(root)
	_ground(root)
	_stage_ring(root)
	_standards(root)
	_torches(root)
	_rocks(root)
	_lanka_skyline(root)
	_mountains(root)


static func _environment(root: Node3D) -> void:
	var sky_mat := ProceduralSkyMaterial.new()
	sky_mat.sky_top_color = Pal.SKY_TOP
	sky_mat.sky_horizon_color = Pal.SKY_HORIZON
	sky_mat.ground_horizon_color = Pal.SKY_HORIZON.darkened(0.3)
	sky_mat.ground_bottom_color = Pal.EARTH_DARK
	sky_mat.sun_angle_max = 12.0
	sky_mat.sky_curve = 0.12
	var sky := Sky.new()
	sky.sky_material = sky_mat

	var env := Environment.new()
	env.background_mode = Environment.BG_SKY
	env.sky = sky
	env.ambient_light_source = Environment.AMBIENT_SOURCE_SKY
	env.ambient_light_energy = 0.9
	env.tonemap_mode = Environment.TONE_MAPPER_FILMIC
	env.tonemap_exposure = 1.05
	env.glow_enabled = true
	env.glow_intensity = 0.7
	env.glow_bloom = 0.08
	env.ssao_enabled = true
	env.fog_enabled = true
	env.fog_light_color = Color(0.72, 0.5, 0.42)
	env.fog_density = 0.008
	env.fog_sky_affect = 0.25
	env.adjustment_enabled = true
	env.adjustment_saturation = 1.08

	var we := WorldEnvironment.new()
	we.environment = env
	root.add_child(we)

	var sun := DirectionalLight3D.new()
	sun.light_color = Pal.SUN
	sun.light_energy = 1.35
	sun.shadow_enabled = true
	sun.directional_shadow_max_distance = 70.0
	sun.rotation_degrees = Vector3(-24, 150, 0)
	root.add_child(sun)


static func _ground(root: Node3D) -> void:
	var body := StaticBody3D.new()
	body.name = "Ground"
	var shape := CollisionShape3D.new()
	shape.shape = WorldBoundaryShape3D.new()
	body.add_child(shape)
	root.add_child(body)

	var noise := FastNoiseLite.new()
	noise.frequency = 0.02
	noise.fractal_octaves = 4
	var tex := NoiseTexture2D.new()
	tex.noise = noise
	tex.seamless = true
	tex.width = 512
	tex.height = 512
	var grad := Gradient.new()
	grad.set_color(0, Pal.EARTH_DARK)
	grad.set_color(1, Pal.EARTH.lightened(0.15))
	tex.color_ramp = grad

	var m := StandardMaterial3D.new()
	m.albedo_texture = tex
	m.roughness = 0.95
	m.uv1_scale = Vector3(24, 24, 1)
	var plane := PlaneMesh.new()
	plane.size = Vector2(260, 260)
	P.part(body, plane, m)


static func _stage_ring(root: Node3D) -> void:
	var gold := P.mat(Pal.GOLD_DEEP, 0.7, 0.4)
	P.part(root, P.torus(ARENA_RADIUS - 0.25, ARENA_RADIUS + 0.25), gold, Vector3(0, 0.02, 0), Vector3.ZERO, Vector3(1, 0.08, 1))
	P.part(root, P.torus(4.8, 5.0), gold, Vector3(0, 0.02, 0), Vector3.ZERO, Vector3(1, 0.05, 1))
	# Eight gilded lotus-bud markers on the ring.
	for i in 8:
		var a := TAU * i / 8.0
		var pos := Vector3(cos(a), 0, sin(a)) * ARENA_RADIUS
		P.part(root, P.cyl(0.25, 0.35, 0.5, 8), P.mat(Pal.STONE, 0.0, 0.9), pos + Vector3(0, 0.25, 0))
		P.part(root, P.sphere(0.22, 0.5), gold, pos + Vector3(0, 0.7, 0))


## ธงชัย (triangular battle flags) and ฉัตร (tiered parasols) ring the field.
static func _standards(root: Node3D) -> void:
	var pole := P.mat(Pal.KHON_RED_DARK, 0.2, 0.6)
	var gold := P.gold()
	for i in 12:
		var a := TAU * (i + 0.5) / 12.0
		var pos := Vector3(cos(a), 0, sin(a)) * (ARENA_RADIUS + 3.5)
		var n := P.pivot(root, pos)
		n.rotation.y = -a
		P.part(n, P.cyl(0.06, 0.08, 7.0, 8), pole, Vector3(0, 3.5, 0))
		P.part(n, P.cone(0.12, 0.5, 8), gold, Vector3(0, 7.25, 0))
		if i % 3 == 0:
			for t in 5:
				var r := 1.1 - t * 0.18
				P.part(n, P.cyl(r * 0.6, r, 0.18, 16), gold if t % 2 == 0 else P.mat(Color.WHITE, 0.0, 0.5), Vector3(0, 4.6 + t * 0.45, 0))
		else:
			var flag_color := Pal.KHON_RED if i % 2 == 0 else Pal.UI_DEPTH
			# Pennant: a thin prism turned so its apex points away from the pole.
			P.part(n, P.prism(Vector3(1.6, 2.2, 0.04)), P.mat(flag_color, 0.0, 0.6), Vector3(1.1, 5.9, 0), Vector3(0, 0, -90))
			P.part(n, P.box(Vector3(0.08, 1.6, 0.06)), gold, Vector3(0.05, 5.9, 0))


static func _torches(root: Node3D) -> void:
	var flame := P.mat(Color(1.0, 0.55, 0.18), 0.0, 0.4, 4.0)
	for i in 6:
		var a := TAU * i / 6.0 + 0.3
		var pos := Vector3(cos(a), 0, sin(a)) * (ARENA_RADIUS + 1.6)
		P.part(root, P.cyl(0.07, 0.1, 2.2, 8), P.mat(Pal.EARTH_DARK, 0.0, 0.9), pos + Vector3(0, 1.1, 0))
		P.part(root, P.cyl(0.25, 0.12, 0.25, 10), P.gold(), pos + Vector3(0, 2.25, 0))
		P.part(root, P.cone(0.18, 0.5, 8), flame, pos + Vector3(0, 2.6, 0))
		var light := OmniLight3D.new()
		light.light_color = Color(1.0, 0.62, 0.3)
		light.light_energy = 2.2
		light.omni_range = 9.0
		light.position = pos + Vector3(0, 2.8, 0)
		root.add_child(light)


static func _rocks(root: Node3D) -> void:
	var rng := RandomNumberGenerator.new()
	rng.seed = 7
	var stone := P.mat(Pal.STONE, 0.0, 0.95)
	for i in 26:
		var a := rng.randf() * TAU
		var d := rng.randf_range(ARENA_RADIUS + 5.0, ARENA_RADIUS + 16.0)
		var s := rng.randf_range(0.5, 2.2)
		P.part(root, P.sphere(1.0), stone, Vector3(cos(a) * d, s * 0.2, sin(a) * d), Vector3(rng.randf() * 40, rng.randf() * 360, 0), Vector3(s, s * 0.55, s * 0.8))


## กรุงลงกา on the northern horizon: a long wall with prang towers.
static func _lanka_skyline(root: Node3D) -> void:
	var wall := P.mat(Color(0.28, 0.2, 0.2), 0.0, 0.9)
	var gold := P.mat(Pal.GOLD_DEEP, 0.6, 0.5)
	var glow := P.mat(Color(1.0, 0.6, 0.25), 0.0, 0.5, 3.0)
	var base := Vector3(0, 0, -95)
	P.part(root, P.box(Vector3(140, 7, 3)), wall, base + Vector3(0, 3.5, 0))
	for i in 15:
		var x := -63.0 + i * 9.0
		P.part(root, P.box(Vector3(2.2, 9, 3.6)), wall, base + Vector3(x, 4.5, 0))
		P.part(root, P.box(Vector3(0.5, 0.8, 0.1)), glow, base + Vector3(x, 5.5, 1.85))
	var prangs := [[-30.0, 26.0], [-12.0, 34.0], [0.0, 46.0], [14.0, 32.0], [32.0, 24.0]]
	for pr in prangs:
		var x: float = pr[0]
		var h: float = pr[1]
		var pos := base + Vector3(x, 0, -10)
		P.part(root, P.box(Vector3(h * 0.3, h * 0.3, h * 0.3)), wall, pos + Vector3(0, h * 0.15, 0))
		P.part(root, P.cyl(h * 0.05, h * 0.14, h * 0.6, 12), wall, pos + Vector3(0, h * 0.6, 0))
		P.part(root, P.cone(h * 0.05, h * 0.25, 12), gold, pos + Vector3(0, h * 1.02, 0))


static func _mountains(root: Node3D) -> void:
	var rng := RandomNumberGenerator.new()
	rng.seed = 11
	var far := P.mat(Color(0.34, 0.28, 0.38), 0.0, 1.0)
	for i in 18:
		var a := TAU * i / 18.0 + rng.randf() * 0.15
		if absf(wrapf(a - 1.5 * PI, -PI, PI)) < 0.55:
			continue  # keep the Lanka view open
		var d := rng.randf_range(150.0, 190.0)
		var w := rng.randf_range(30.0, 55.0)
		var h := rng.randf_range(10.0, 22.0)
		# Low rounded hills (half-buried spheroids) read as distance, not pyramids.
		P.part(root, P.sphere(1.0), far, Vector3(cos(a) * d, -h * 0.25, sin(a) * d), Vector3(0, rng.randf() * 360.0, 0), Vector3(w, h, w * 0.7))
