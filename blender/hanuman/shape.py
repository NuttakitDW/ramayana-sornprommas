"""Hanuman's lacquered head shell: an SDF sculpt meshed with surface nets.

Spec (lore/khon-performance/05-masks-costumes.md §2.4): white face, bald head
with a gold coronet, open mouth (ปากอ้า) with a crystal fang on the palate.
Units are metres; the face looks down -Y, Z is up.
"""
import numpy as np

from khon import geometry as geo
from khon import meshing
from khon import sdf

BOUNDS_LO = (-0.20, -0.30, -0.19)
BOUNDS_HI = (0.20, 0.22, 0.28)
BOTTOM_Z = -0.17

MOUTH = {"center": (0.0, -0.17, -0.098), "radii": (0.062, 0.10, 0.030)}
NOSTRIL = {"center": (0.021, -0.213, -0.036), "radii": (0.008, 0.012, 0.006)}

# Material slots on the shell, in order.
WHITE, LIPS, MOUTH_IN, BLACK = range(4)
SHELL_MATERIALS = ("lacquer_white", "lacquer_red", "mouth_red", "lacquer_black")


def shell_sdf(p):
    m = sdf.mirror_x(p)
    d = sdf.ellipsoid(p, (0.0, 0.03, 0.07), (0.12, 0.13, 0.14))                      # cranium
    d = sdf.smooth_union(d, sdf.ellipsoid(p, (0.0, 0.03, -0.06), (0.11, 0.12, 0.12)), 0.04)  # nape
    d = sdf.smooth_union(d, sdf.ellipsoid(p, (0.0, -0.035, -0.01), (0.118, 0.10, 0.12)), 0.04)  # face
    d = sdf.smooth_union(d, sdf.ellipsoid(p, (0.0, -0.105, 0.058), (0.105, 0.035, 0.028),
                                          sdf.rotation(x_deg=-10)), 0.025)            # brow ridge
    d = sdf.smooth_union(d, sdf.ellipsoid(m, (0.06, -0.10, -0.025), (0.05, 0.045, 0.045)), 0.03)  # cheeks
    d = sdf.smooth_union(d, sdf.ellipsoid(p, (0.0, -0.135, -0.06), (0.085, 0.075, 0.055)), 0.03)  # muzzle
    d = sdf.smooth_union(d, sdf.ellipsoid(p, (0.0, -0.115, -0.135), (0.07, 0.07, 0.035),
                                          sdf.rotation(x_deg=12)), 0.03)              # dropped jaw
    d = sdf.smooth_union(d, sdf.ellipsoid(p, (0.0, -0.20, -0.03), (0.03, 0.022, 0.018)), 0.015)  # nose pad
    d = sdf.smooth_union(d, sdf.ellipsoid(m, (0.135, 0.03, 0.0), (0.022, 0.035, 0.05),
                                          sdf.rotation(z_deg=-15)), 0.012)            # ears
    d = sdf.smooth_subtract(d, sdf.ellipsoid(p, **MOUTH), 0.012)
    d = sdf.smooth_subtract(d, sdf.ellipsoid(m, **NOSTRIL), 0.004)
    return sdf.intersect(d, sdf.above(p, BOTTOM_Z))


def build_shell(mats, parent, spacing):
    field, origin = meshing.sample(shell_sdf, BOUNDS_LO, BOUNDS_HI, spacing)
    verts, quads = meshing.surface_nets(field, origin, spacing)
    obj = geo.mesh_object("Shell", verts, quads, None, parent)
    geo.relax_mesh(obj)
    for name in SHELL_MATERIALS:
        obj.data.materials.append(mats[name])
    obj.data.shade_smooth()
    return obj


def paint_regions(obj, eye_centers, eye_radius):
    """Assign lips / mouth / eye-ring / nostril materials per face."""
    polys = obj.data.polygons
    centers = np.empty(len(polys) * 3, np.float32)
    polys.foreach_get("center", centers)
    centers = centers.reshape(-1, 3)

    mouth = sdf.ellipsoid(centers, **MOUTH)
    nostril = sdf.ellipsoid(sdf.mirror_x(centers), **NOSTRIL)
    region = np.full(len(centers), WHITE, np.int32)
    region[(mouth < 0.013) & (centers[:, 1] < -0.15)] = LIPS
    region[mouth < 0.004] = MOUTH_IN
    for c in eye_centers:
        region[np.linalg.norm(centers - c, axis=1) < eye_radius * 1.5] = BLACK
    region[nostril < 0.003] = BLACK
    polys.foreach_set("material_index", region)
    obj.data.update()
