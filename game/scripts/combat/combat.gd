extends RefCounted
## Shared combat helpers. Anything that can be struck joins group "hittable"
## and implements:
##   is_alive() -> bool
##   hurt_points() -> Array[Vector3]     world-space points that can be struck
##   hurt_radius() -> float
##   receive_hit(damage, from_pos, knockback, point_index) -> bool

const Arena := preload("res://scripts/world/arena_builder.gd")


## Returns [{"target": Node, "index": int, "point": Vector3}] for every hittable
## whose nearest hurt point lies inside a forward arc from `origin`.
static func sweep(tree: SceneTree, origin: Vector3, forward: Vector3, reach: float, arc_deg: float, vertical: float) -> Array:
	var found: Array = []
	var fwd := Vector3(forward.x, 0, forward.z).normalized()
	var half := deg_to_rad(arc_deg) * 0.5
	for target in tree.get_nodes_in_group("hittable"):
		if not target.is_alive():
			continue
		var best_i := -1
		var best_d := INF
		var points: Array = target.hurt_points()
		for i in points.size():
			var pt: Vector3 = points[i]
			var to := pt - origin
			if absf(to.y) > vertical:
				continue
			var flat := Vector3(to.x, 0, to.z)
			var d := flat.length()
			if d > reach + target.hurt_radius():
				continue
			if d > 0.9 and fwd.angle_to(flat) > half:
				continue
			if d < best_d:
				best_d = d
				best_i = i
		if best_i >= 0:
			found.append({"target": target, "index": best_i, "point": points[best_i]})
	return found


## Nearest live hurt point within `max_dist` (for soft auto-aim / AI).
static func nearest_point(tree: SceneTree, origin: Vector3, max_dist: float, vertical := 2.5) -> Dictionary:
	var best := {}
	var best_d := max_dist
	for target in tree.get_nodes_in_group("hittable"):
		if not target.is_alive():
			continue
		for pt in target.hurt_points():
			var to: Vector3 = pt - origin
			if absf(to.y) > vertical:
				continue
			var d := Vector2(to.x, to.z).length()
			if d < best_d:
				best_d = d
				best = {"target": target, "point": pt, "dist": d}
	return best


static func clamp_to_arena(body: Node3D, margin := 0.0) -> void:
	var r := Arena.ARENA_RADIUS - margin
	var flat := Vector2(body.global_position.x, body.global_position.z)
	if flat.length() > r:
		flat = flat.normalized() * r
		body.global_position = Vector3(flat.x, body.global_position.y, flat.y)


static func yaw_toward(dir: Vector3) -> float:
	return atan2(-dir.x, -dir.z)
