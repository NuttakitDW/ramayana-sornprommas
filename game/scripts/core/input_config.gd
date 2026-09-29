extends RefCounted
## Registers all input actions at runtime so the project needs no editor setup.
## Keyboard + mouse and gamepad are both supported.

const DEADZONE := 0.2


static func register() -> void:
	if InputMap.has_action("attack"):
		return  # already registered (scene reload)
	_keys("move_forward", [KEY_W])
	_keys("move_back", [KEY_S])
	_keys("move_left", [KEY_A])
	_keys("move_right", [KEY_D])
	_joy_axis("move_forward", JOY_AXIS_LEFT_Y, -1.0)
	_joy_axis("move_back", JOY_AXIS_LEFT_Y, 1.0)
	_joy_axis("move_left", JOY_AXIS_LEFT_X, -1.0)
	_joy_axis("move_right", JOY_AXIS_LEFT_X, 1.0)

	_keys("cam_left", [KEY_LEFT])
	_keys("cam_right", [KEY_RIGHT])
	_joy_axis("cam_left", JOY_AXIS_RIGHT_X, -1.0)
	_joy_axis("cam_right", JOY_AXIS_RIGHT_X, 1.0)

	_keys("attack", [KEY_J])
	_mouse("attack", MOUSE_BUTTON_LEFT)
	_joy_button("attack", JOY_BUTTON_X)

	_keys("heavy", [KEY_K])
	_mouse("heavy", MOUSE_BUTTON_RIGHT)
	_joy_button("heavy", JOY_BUTTON_Y)

	_keys("jump", [KEY_SPACE])
	_joy_button("jump", JOY_BUTTON_A)

	_keys("dash", [KEY_SHIFT, KEY_L])
	_joy_button("dash", JOY_BUTTON_B)

	_keys("special", [KEY_Q])
	_joy_button("special", JOY_BUTTON_RIGHT_SHOULDER)

	_keys("confirm", [KEY_ENTER, KEY_SPACE, KEY_J])
	_mouse("confirm", MOUSE_BUTTON_LEFT)
	_joy_button("confirm", JOY_BUTTON_A)

	_keys("restart", [KEY_R])
	_joy_button("restart", JOY_BUTTON_START)

	_keys("toggle_help", [KEY_H])
	_keys("release_mouse", [KEY_ESCAPE])


static func _ensure(action: StringName) -> void:
	if not InputMap.has_action(action):
		InputMap.add_action(action, DEADZONE)


static func _keys(action: StringName, keycodes: Array) -> void:
	_ensure(action)
	for code in keycodes:
		var ev := InputEventKey.new()
		ev.physical_keycode = code
		InputMap.action_add_event(action, ev)


static func _mouse(action: StringName, button: MouseButton) -> void:
	_ensure(action)
	var ev := InputEventMouseButton.new()
	ev.button_index = button
	InputMap.action_add_event(action, ev)


static func _joy_button(action: StringName, button: JoyButton) -> void:
	_ensure(action)
	var ev := InputEventJoypadButton.new()
	ev.button_index = button
	InputMap.action_add_event(action, ev)


static func _joy_axis(action: StringName, axis: JoyAxis, value: float) -> void:
	_ensure(action)
	var ev := InputEventJoypadMotion.new()
	ev.axis = axis
	ev.axis_value = value
	InputMap.action_add_event(action, ev)
