"""Mesh and curve construction helpers shared by the Khon mask builders."""
from collections import defaultdict

import bmesh
import bpy
import numpy as np
from mathutils import Vector
from mathutils.bvhtree import BVHTree


# ---------------------------------------------------------------- math

def normalize(v):
    v = np.asarray(v, float)
    return v / max(np.linalg.norm(v), 1e-12)


def frame(z_axis, up=(0.0, 0.0, 1.0)):
    """Rotation whose local Z is z_axis, with local Y leaning towards up."""
    z = normalize(z_axis)
    up = normalize(up)
    if abs(np.dot(z, up)) > 0.95:
        up = np.array([0.0, 1.0, 0.0])
    x = normalize(np.cross(up, z))
    return np.stack([x, np.cross(z, x), z], axis=1)


def transform(rot=None, loc=(0.0, 0.0, 0.0), scale=(1.0, 1.0, 1.0)):
    m = np.eye(4)
    m[:3, :3] = (np.eye(3) if rot is None else rot) @ np.diag(scale)
    m[:3, 3] = loc
    return m


def relax_polyline(pts, iterations, closed):
    """Laplacian smoothing along a polyline; open ends stay pinned."""
    pts = np.asarray(pts, float)
    for _ in range(iterations):
        if closed:
            pts = 0.5 * pts + 0.25 * (np.roll(pts, 1, 0) + np.roll(pts, -1, 0))
        elif len(pts) > 2:
            inner = 0.5 * pts[1:-1] + 0.25 * (pts[:-2] + pts[2:])
            pts = np.concatenate([pts[:1], inner, pts[-1:]])
    return pts


# ---------------------------------------------------------------- templates

def _bm_arrays(bm):
    verts = np.array([v.co[:] for v in bm.verts], float)
    faces = [[v.index for v in f.verts] for f in bm.faces]
    bm.free()
    return verts, faces


def uv_sphere(radius, segments=16, rings=8):
    bm = bmesh.new()
    bmesh.ops.create_uvsphere(bm, u_segments=segments, v_segments=rings, radius=radius)
    return _bm_arrays(bm)


def cone(radius_base, radius_tip, depth, segments=12):
    """Cone along +Z, base at -depth/2."""
    bm = bmesh.new()
    bmesh.ops.create_cone(bm, cap_ends=True, cap_tris=False, segments=segments,
                          radius1=radius_base, radius2=radius_tip, depth=depth)
    return _bm_arrays(bm)


def box(sx, sy, sz):
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    verts, faces = _bm_arrays(bm)
    return verts * np.array([sx, sy, sz]), faces


def torus(major, minor, segments=32, rings=10):
    """Torus in the local XY plane around +Z."""
    u = np.linspace(0, 2 * np.pi, segments, endpoint=False)
    v = np.linspace(0, 2 * np.pi, rings, endpoint=False)
    uu, vv = np.meshgrid(u, v, indexing="ij")
    rad = major + minor * np.cos(vv)
    verts = np.stack([rad * np.cos(uu), rad * np.sin(uu), minor * np.sin(vv)], -1).reshape(-1, 3)
    faces = []
    for i in range(segments):
        for j in range(rings):
            i2, j2 = (i + 1) % segments, (j + 1) % rings
            faces.append([i * rings + j, i2 * rings + j, i2 * rings + j2, i * rings + j2])
    return verts, faces


def slab(outline, thickness):
    """Extrude a convex 2D outline (XY) into a plate centred on z=0."""
    outline = np.asarray(outline, float)
    n = len(outline)
    front = np.column_stack([outline, np.full(n, thickness / 2)])
    back = np.column_stack([outline, np.full(n, -thickness / 2)])
    faces = [list(range(n)), list(range(2 * n - 1, n - 1, -1))]
    faces += [[i, n + i, n + (i + 1) % n, (i + 1) % n] for i in range(n)]
    return np.concatenate([front, back]), faces


def instances(template, matrices):
    """Bake copies of a template mesh into one vertex/face list."""
    tv, tf = template
    verts, faces, offset = [], [], 0
    for m in matrices:
        verts.append(tv @ m[:3, :3].T + m[:3, 3])
        faces.extend([[i + offset for i in f] for f in tf])
        offset += len(tv)
    return np.concatenate(verts), faces


# ---------------------------------------------------------------- objects

def link(obj, parent=None):
    bpy.context.scene.collection.objects.link(obj)
    if parent is not None:
        obj.parent = parent
    return obj


def mesh_object(name, verts, faces, material=None, parent=None, smooth=True):
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(np.asarray(verts, float).tolist(), [], [list(map(int, f)) for f in faces])
    mesh.validate()
    mesh.update()
    if smooth:
        mesh.shade_smooth()
    if material is not None:
        mesh.materials.append(material)
    return link(bpy.data.objects.new(name, mesh), parent)


def tube_object(name, polylines, radius, material, parent=None, closed=None):
    """Round gold-relief style tubes following polylines."""
    curve = bpy.data.curves.new(name, "CURVE")
    curve.dimensions = "3D"
    curve.bevel_depth = radius
    curve.bevel_resolution = 3
    curve.use_fill_caps = True
    for i, pts in enumerate(polylines):
        spline = curve.splines.new("POLY")
        spline.points.add(len(pts) - 1)
        co = np.column_stack([pts, np.ones(len(pts))]).ravel()
        spline.points.foreach_set("co", co.tolist())
        spline.use_cyclic_u = bool(closed[i]) if closed else False
        spline.use_smooth = True
    curve.materials.append(material)
    return link(bpy.data.objects.new(name, curve), parent)


def relax_mesh(obj, iterations=6, factor=0.5):
    """Drop loose verts, smooth the surface and make normals point outward."""
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    loose = [v for v in bm.verts if not v.link_faces]
    bmesh.ops.delete(bm, geom=loose, context="VERTS")
    for _ in range(iterations):
        bmesh.ops.smooth_vert(bm, verts=bm.verts, factor=factor,
                              use_axis_x=True, use_axis_y=True, use_axis_z=True)
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    bm.to_mesh(obj.data)
    bm.free()
    obj.data.update()


def bvh_of(obj):
    return BVHTree.FromObject(obj, bpy.context.evaluated_depsgraph_get())


def ray(bvh, origin, direction):
    hit, normal, _, _ = bvh.ray_cast(Vector(origin), Vector(direction).normalized())
    if hit is None:
        return None, None
    return np.array(hit), np.array(normal)


def snap(bvh, pts, offset):
    """Move points onto the surface, lifted along the surface normal."""
    out = []
    for p in pts:
        loc, normal, _, _ = bvh.find_nearest(Vector(p))
        out.append(np.array(loc) + np.array(normal) * offset)
    return np.array(out)


def material_boundaries(obj, keep):
    """Polylines along edges where face materials change; keep(a, b) filters pairs."""
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    bm.verts.ensure_lookup_table()
    adj = defaultdict(list)
    for e in bm.edges:
        faces = e.link_faces
        if len(faces) != 2:
            continue
        a, b = faces[0].material_index, faces[1].material_index
        if a != b and keep(a, b):
            u, v = e.verts[0].index, e.verts[1].index
            adj[u].append(v)
            adj[v].append(u)
    coords = {i: np.array(bm.verts[i].co) for i in adj}
    bm.free()
    return [(np.array([coords[i] for i in chain]), closed) for chain, closed in _walk_chains(adj)]


def _walk_chains(adj):
    seen = set()

    def walk(start, nxt):
        chain, prev, cur = [start, nxt], start, nxt
        seen.add(frozenset((start, nxt)))
        while len(adj[cur]) == 2:
            a, b = adj[cur]
            step = b if a == prev else a
            key = frozenset((cur, step))
            if key in seen:
                break
            seen.add(key)
            chain.append(step)
            prev, cur = cur, step
        return chain

    chains = []
    ordered = sorted(adj, key=lambda v: len(adj[v]) == 2)  # endpoints/junctions first
    for v in ordered:
        for n in adj[v]:
            if frozenset((v, n)) not in seen:
                chain = walk(v, n)
                closed = len(chain) > 3 and chain[0] == chain[-1]
                chains.append((chain[:-1] if closed else chain, closed))
    return chains


def bake_to_meshes(root):
    """Replace curves and modified meshes under root with plain meshes (for export)."""
    depsgraph = bpy.context.evaluated_depsgraph_get()
    for obj in list(root.children):
        if obj.type == "MESH" and not obj.modifiers:
            continue
        if obj.type not in {"MESH", "CURVE"}:
            continue
        mesh = bpy.data.meshes.new_from_object(obj.evaluated_get(depsgraph))
        mesh.shade_smooth()
        name = obj.name
        bpy.data.objects.remove(obj)
        baked = bpy.data.objects.new(name, mesh)
        link(baked, root)
