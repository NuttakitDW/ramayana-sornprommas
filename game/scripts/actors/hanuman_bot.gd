extends RefCounted
## Simple auto-pilot for Hanuman, used for automated play-testing
## (run with `-- --autobot`). Walks to the nearest reachable target and attacks.

const Combat := preload("res://scripts/combat/combat.gd")

var body: CharacterBody3D
var _presses: Dictionary = {}
var _move := Vector2.ZERO
var _rng := RandomNumberGenerator.new()


func _init(owner_body: CharacterBody3D) -> void:
	body = owner_body
	_rng.seed = 42


func pressed(action: String) -> bool:
	if _presses.get(action, false):
		_presses[action] = false
		return true
	return false


func move() -> Vector2:
	return _move


func think(_delta: float) -> void:
	_presses = {}
	_move = Vector2.ZERO
	var tree := body.get_tree()
	var cam := tree.get_first_node_in_group("camera_rig")
	if cam == null:
		return
	if body.power >= 100.0 and body.giant_left <= 0.0:
		_presses["special"] = true
		return
	var target := Combat.nearest_point(tree, body.global_position + Vector3(0, 1, 0), 80.0, 3.2)
	var goal := Vector3.ZERO
	if not target.is_empty():
		goal = target["point"]
	var to := goal - body.global_position
	to.y = 0
	var dist := to.length()
	if target.is_empty() and dist < 3.0:
		return
	if dist > 2.3 or target.is_empty():
		var d := to.normalized()
		_move = Vector2(d.dot(cam.right_flat()), -d.dot(cam.forward_flat()))
		if dist > 8.0 and _rng.randf() < 0.01:
			_presses["dash"] = true
	else:
		_presses["heavy" if _rng.randf() < 0.12 else "attack"] = true
	if _rng.randf() < 0.004:
		_presses["jump"] = true
