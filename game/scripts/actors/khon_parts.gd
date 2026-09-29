extends RefCounted
## Primitive mesh + material helpers used to assemble Khon-style figures
## without any imported assets.

static var _mat_cache: Dictionary = {}


static func mat(color: Color, metallic := 0.0, roughness := 0.7, emission_energy := 0.0) -> StandardMaterial3D:
	var key := "%s|%.2f|%.2f|%.2f" % [color.to_html(), metallic, roughness, emission_energy]
	if _mat_cache.has(key):
		return _mat_cache[key]
	var m := StandardMaterial3D.new()
	m.albedo_color = color
	m.metallic = metallic
	m.roughness = roughness
	if emission_energy > 0.0:
		m.emission_enabled = true
		m.emission = color
		m.emission_energy_multiplier = emission_energy
	_mat_cache[key] = m
	return m


static func gold() -> StandardMaterial3D:
	return mat(Color(0.86, 0.66, 0.24), 0.85, 0.32)


static func part(parent: Node3D, mesh: Mesh, material: Material, pos := Vector3.ZERO, rot_deg := Vector3.ZERO, scale := Vector3.ONE) -> MeshInstance3D:
	var mi := MeshInstance3D.new()
	mi.mesh = mesh
	mi.material_override = material
	mi.position = pos
	mi.rotation_degrees = rot_deg
	mi.scale = scale
	parent.add_child(mi)
	return mi


static func pivot(parent: Node3D, pos: Vector3, node_name := "") -> Node3D:
	var n := Node3D.new()
	if node_name != "":
		n.name = node_name
	n.position = pos
	parent.add_child(n)
	return n


static func box(size: Vector3) -> BoxMesh:
	var m := BoxMesh.new()
	m.size = size
	return m


static func sphere(radius: float, height := -1.0) -> SphereMesh:
	var m := SphereMesh.new()
	m.radius = radius
	m.height = radius * 2.0 if height < 0.0 else height
	m.radial_segments = 24
	m.rings = 12
	return m


static func capsule(radius: float, height: float) -> CapsuleMesh:
	var m := CapsuleMesh.new()
	m.radius = radius
	m.height = maxf(height, radius * 2.0)
	m.radial_segments = 16
	m.rings = 6
	return m


static func cyl(top_radius: float, bottom_radius: float, height: float, segments := 16) -> CylinderMesh:
	var m := CylinderMesh.new()
	m.top_radius = top_radius
	m.bottom_radius = bottom_radius
	m.height = height
	m.radial_segments = segments
	m.rings = 1
	return m


static func cone(radius: float, height: float, segments := 16) -> CylinderMesh:
	return cyl(0.0, radius, height, segments)


static func torus(inner_radius: float, outer_radius: float) -> TorusMesh:
	var m := TorusMesh.new()
	m.inner_radius = inner_radius
	m.outer_radius = outer_radius
	m.rings = 24
	m.ring_segments = 10
	return m


static func prism(size: Vector3) -> PrismMesh:
	var m := PrismMesh.new()
	m.size = size
	return m


## Collects every MeshInstance3D under a node (used for hit-flash overlays).
static func collect_meshes(root: Node) -> Array[MeshInstance3D]:
	var out: Array[MeshInstance3D] = []
	var stack: Array[Node] = [root]
	while not stack.is_empty():
		var n: Node = stack.pop_back()
		if n is MeshInstance3D:
			out.append(n)
		for c in n.get_children():
			stack.append(c)
	return out
