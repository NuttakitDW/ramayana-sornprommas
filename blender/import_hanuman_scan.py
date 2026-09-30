"""Clean up the photogrammetry scan of a Hanuman Khon mask and export it for Unreal.

    blender -b -P blender/import_hanuman_scan.py -- [--render]

Source: "head hanuman" by tavi2516vit1, CC BY 4.0
https://sketchfab.com/3d-models/44f77a9c04bc420e8a6236380795fc2c
Unzip the Sketchfab download into blender/scans/head-hanuman/ first.

Output: blender/build/scans/SM_HanumanMask.glb — origin at the centre of the
neck opening, face towards -Y (Blender), sized for the game's Khon figures.
"""
import argparse
import os
import sys
from dataclasses import dataclass

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import bmesh  # noqa: E402
import bpy  # noqa: E402
import numpy as np  # noqa: E402
from mathutils import Matrix  # noqa: E402

from khon import stage  # noqa: E402


@dataclass(frozen=True)
class ScanSpec:
    name: str
    source: str
    textures: str
    drop: tuple
    height_m: float      # final height, crown tip to neck opening
    trim_bottom: float   # fraction of height sliced off the ragged scan edge


HANUMAN = ScanSpec(
    name="SM_HanumanMask",
    source="scans/head-hanuman/source/hanuman.fbx",
    textures="scans/head-hanuman/textures",
    drop=("Lathe",),     # the wooden display stand
    height_m=0.60,
    trim_bottom=0.025,
)


def import_source(spec: ScanSpec) -> bpy.types.Object:
    bpy.ops.import_scene.fbx(filepath=os.path.join(HERE, spec.source))
    bpy.context.view_layer.update()
    meshes = [o for o in bpy.data.objects if o.type == "MESH" and o.name not in spec.drop]
    if len(meshes) != 1:
        raise RuntimeError(f"expected one scan mesh, found {[o.name for o in meshes]}")
    obj = meshes[0]
    world = obj.matrix_world.copy()
    obj.parent = None
    obj.data.transform(world)          # bake the FBX parent's scale/rotation into the mesh
    obj.matrix_world = Matrix.Identity(4)
    for name in spec.drop:
        bpy.data.objects.remove(bpy.data.objects[name])
    return obj


def _islands(bm: bmesh.types.BMesh) -> list:
    seen, parts = set(), []
    for v in bm.verts:
        if v.index in seen:
            continue
        stack, part = [v], []
        seen.add(v.index)
        while stack:
            cur = stack.pop()
            part.append(cur)
            for e in cur.link_edges:
                other = e.other_vert(cur)
                if other.index not in seen:
                    seen.add(other.index)
                    stack.append(other)
        parts.append(part)
    return sorted(parts, key=len, reverse=True)


def clean(obj: bpy.types.Object, spec: ScanSpec) -> None:
    """Keep the mask shell only: drop the stand's post island and the ragged bottom rim."""
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    bm.verts.ensure_lookup_table()
    stray = [v for part in _islands(bm)[1:] for v in part]
    z = np.array([v.co.z for v in bm.verts])
    cut = z.min() + spec.trim_bottom * (z.max() - z.min())
    rim = [v for v in bm.verts if v.co.z < cut]
    doomed = list({v.index: v for v in stray + rim}.values())
    bmesh.ops.delete(bm, geom=doomed, context="VERTS")
    bm.to_mesh(obj.data)
    bm.free()
    print(f"[scan] removed {len(stray)} stand verts and {len(rim)} rim verts")


def normalize(obj: bpy.types.Object, spec: ScanSpec) -> None:
    """Scale to height_m and put the origin at the centre of the neck opening."""
    co = np.empty(len(obj.data.vertices) * 3)
    obj.data.vertices.foreach_get("co", co)
    co = co.reshape(-1, 3)
    lo, hi = co.min(0), co.max(0)
    s = spec.height_m / (hi[2] - lo[2])
    base = np.array([(lo[0] + hi[0]) / 2, (lo[1] + hi[1]) / 2, lo[2]])
    obj.data.transform(Matrix.Scale(s, 4) @ Matrix.Translation(-base))
    obj.name = obj.data.name = spec.name
    dims = (hi - lo) * s
    print(f"[scan] {spec.name}: {dims[0]:.3f} x {dims[1]:.3f} x {dims[2]:.3f} m")


def relink_textures(spec: ScanSpec) -> None:
    folder = os.path.join(HERE, spec.textures)
    for img in bpy.data.images:
        path = os.path.join(folder, os.path.basename(img.filepath))
        if os.path.exists(path):
            img.filepath = path
            img.reload()


def brighten_textures(white_point: float = 0.95, gamma: float = 0.8, desaturate: float = 0.3) -> None:
    """Lift the scan's grey photo exposure so Hanuman reads white (ขาว) under game light.

    Maps the 90th-percentile luminance to white_point, lifts mid-tones with gamma
    and pulls the scan's green cast part-way to grey, keeping some pearl shimmer.
    """
    for img in bpy.data.images:
        if img.size[0] == 0:
            continue
        px = np.empty(img.size[0] * img.size[1] * 4, np.float32)
        img.pixels.foreach_get(px)
        rgba = px.reshape(-1, 4)
        luma = rgba[:, :3] @ np.array([0.2126, 0.7152, 0.0722], np.float32)
        gain = white_point / max(float(np.percentile(luma, 90)), 1e-3)
        rgb = rgba[:, :3] + (luma[:, None] - rgba[:, :3]) * desaturate
        rgba[:, :3] = np.clip(rgb * gain, 0.0, 1.0) ** gamma
        img.pixels.foreach_set(rgba.ravel())
        img.update()
        print(f"[scan] brightened {img.name}: gain {gain:.2f}")


def tune_materials(obj: bpy.types.Object) -> None:
    """Satin lacquer / mother-of-pearl, neutral specular."""
    for mat in obj.data.materials:
        bsdf = next(n for n in mat.node_tree.nodes if n.type == "BSDF_PRINCIPLED")
        bsdf.inputs["Metallic"].default_value = 0.0
        bsdf.inputs["Roughness"].default_value = 0.38
        # The FBX sets specular to 1.0 (exported as a 2x glTF specular factor),
        # which Unreal renders as dark polished metal.
        bsdf.inputs["Specular IOR Level"].default_value = 0.5
        bsdf.inputs["Specular Tint"].default_value = (1.0, 1.0, 1.0, 1.0)
        bsdf.inputs["Coat Weight"].default_value = 0.0


def export(obj: bpy.types.Object, out_dir: str) -> str:
    os.makedirs(out_dir, exist_ok=True)
    path = os.path.join(out_dir, f"{obj.name}.glb")
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.ops.export_scene.gltf(filepath=path, export_format="GLB", use_selection=True,
                              export_image_format="JPEG", export_jpeg_quality=92)
    return path


def main() -> None:
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    ap = argparse.ArgumentParser()
    ap.add_argument("--render", action="store_true")
    ap.add_argument("--out", default=os.path.join(HERE, "build", "scans"))
    args = ap.parse_args(argv)

    bpy.ops.wm.read_factory_settings(use_empty=True)
    spec = HANUMAN
    obj = import_source(spec)
    clean(obj, spec)
    normalize(obj, spec)
    relink_textures(spec)
    brighten_textures()
    tune_materials(obj)
    obj.data.shade_smooth()
    print(f"[scan] wrote {export(obj, args.out)}")

    if args.render:
        look_at = (0.0, 0.0, spec.height_m * 0.5)
        cam = stage.setup(look_at, samples=64)
        views = [("hanuman_scan_front", 0, 4), ("hanuman_scan_three_quarter", -35, 10)]
        for path in stage.render_views(cam, look_at, os.path.join(args.out, "renders"), views, distance=2.4):
            print(f"[scan] rendered {path}")


if __name__ == "__main__":
    main()
