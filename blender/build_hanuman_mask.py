"""Build Hanuman's Khon mask (หัวโขนหนุมาน) procedurally and export it for Unreal.

    blender -b -P blender/build_hanuman_mask.py -- [--render] [--out DIR] [--spacing 0.002]

Writes DIR/hanuman_mask.blend, DIR/hanuman_mask.glb and, with --render,
DIR/renders/{front,three_quarter,profile}.png.
"""
import argparse
import os
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import bpy  # noqa: E402

from hanuman import coronet, features, shape  # noqa: E402
from khon import geometry, materials, stage  # noqa: E402

LOOK_AT = (0.0, -0.03, 0.04)
VIEWS = [("front", 0, 4), ("three_quarter", -35, 10), ("profile", -90, 4)]


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--out", default=os.path.join(HERE, "build"))
    ap.add_argument("--render", action="store_true")
    ap.add_argument("--spacing", type=float, default=0.002, help="SDF voxel size in metres")
    ap.add_argument("--samples", type=int, default=96)
    return ap.parse_args(argv)


def build(spacing):
    mats = materials.khon_palette()
    root = geometry.link(bpy.data.objects.new("HanumanMask", None))

    shell = shape.build_shell(mats, root, spacing)
    bvh = geometry.bvh_of(shell)
    eyes = features.eye_anchors(bvh)
    shape.paint_regions(shell, [e["center"] for e in eyes], features.EYE_R)

    features.outline_regions(shell, bvh, mats, root)
    features.add_eyes(eyes, mats, root)
    features.add_brows_and_curls(bvh, mats, root)
    features.add_mouth(bvh, mats, root)
    features.add_ear_rings(bvh, mats, root)
    coronet.add_coronet(bvh, mats, root)
    geometry.bake_to_meshes(root)
    return root


def export(out_dir):
    blend = os.path.join(out_dir, "hanuman_mask.blend")
    glb = os.path.join(out_dir, "hanuman_mask.glb")
    bpy.ops.wm.save_as_mainfile(filepath=blend)
    bpy.ops.export_scene.gltf(filepath=glb, export_format="GLB", export_apply=True)
    return blend, glb


def main():
    args = parse_args()
    os.makedirs(args.out, exist_ok=True)
    bpy.ops.wm.read_factory_settings(use_empty=True)

    started = time.time()
    root = build(args.spacing)
    faces = sum(len(o.data.polygons) for o in root.children if o.type == "MESH")
    print(f"[hanuman] built {len(root.children)} parts, {faces} faces in {time.time() - started:.1f}s")

    for path in export(args.out):
        print(f"[hanuman] wrote {path}")
    if args.render:
        cam = stage.setup(LOOK_AT, samples=args.samples)
        for path in stage.render_views(cam, LOOK_AT, os.path.join(args.out, "renders"), VIEWS):
            print(f"[hanuman] rendered {path}")


if __name__ == "__main__":
    main()
