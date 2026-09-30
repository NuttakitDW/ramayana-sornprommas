"""Turntable-style preview renders: dark stage, warm key, cool rim (Cycles on Metal)."""
import os

import bpy
import numpy as np

BACKDROP = (0.0025, 0.0032, 0.006)  # near-black, blue-biased


def _use_gpu(scene):
    try:
        prefs = bpy.context.preferences.addons["cycles"].preferences
        prefs.compute_device_type = "METAL"
        prefs.get_devices()
        for device in prefs.devices:
            device.use = True
        scene.cycles.device = "GPU"
    except (KeyError, TypeError, AttributeError):
        scene.cycles.device = "CPU"


def _area_light(name, loc, target, power, color, size):
    light = bpy.data.lights.new(name, "AREA")
    light.energy = power
    light.color = color
    light.size = size
    obj = bpy.data.objects.new(name, light)
    bpy.context.scene.collection.objects.link(obj)
    obj.location = loc
    track = obj.constraints.new("TRACK_TO")
    track.target = target
    track.track_axis = "TRACK_NEGATIVE_Z"
    track.up_axis = "UP_Y"
    return obj


def setup(target_loc, resolution=1200, samples=96):
    scene = bpy.context.scene
    scene.render.engine = "CYCLES"
    _use_gpu(scene)
    scene.cycles.samples = samples
    scene.cycles.use_denoising = True
    scene.render.resolution_x = scene.render.resolution_y = resolution
    scene.render.film_transparent = False

    world = bpy.data.worlds.new("Stage")
    scene.world = world
    if world.node_tree is None:
        world.use_nodes = True
    background = next(n for n in world.node_tree.nodes if n.type == "BACKGROUND")
    background.inputs["Color"].default_value = (*BACKDROP, 1.0)
    background.inputs["Strength"].default_value = 1.0

    target = bpy.data.objects.new("LookAt", None)
    scene.collection.objects.link(target)
    target.location = target_loc
    _area_light("Key", (-0.75, -0.95, 0.75), target, 90.0, (1.0, 0.88, 0.74), 0.6)
    _area_light("Fill", (0.95, -0.7, 0.15), target, 22.0, (0.78, 0.84, 1.0), 0.8)
    _area_light("Rim", (0.35, 0.95, 0.8), target, 120.0, (0.55, 0.68, 1.0), 0.4)

    cam = bpy.data.objects.new("Camera", bpy.data.cameras.new("Camera"))
    scene.collection.objects.link(cam)
    cam.data.lens = 85
    track = cam.constraints.new("TRACK_TO")
    track.target = target
    track.track_axis = "TRACK_NEGATIVE_Z"
    track.up_axis = "UP_Y"
    scene.camera = cam
    return cam


def render_views(cam, target_loc, out_dir, views, distance=1.35):
    """views: [(name, azimuth_deg, elevation_deg)]; azimuth 0 looks at the face (-Y)."""
    os.makedirs(out_dir, exist_ok=True)
    scene = bpy.context.scene
    paths = []
    for name, az, el in views:
        a, e = np.radians(az), np.radians(el)
        offset = distance * np.array([np.sin(a) * np.cos(e), -np.cos(a) * np.cos(e), np.sin(e)])
        cam.location = np.asarray(target_loc) + offset
        scene.render.filepath = os.path.join(out_dir, f"{name}.png")
        bpy.ops.render.render(write_still=True)
        paths.append(scene.render.filepath)
    return paths
