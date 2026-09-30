"""Hanuman's face details: bulging eyes, gold relief curls, glass inlay, mouth, ear rings."""
import numpy as np

from khon import geometry as geo
from hanuman import shape

EYE_X, EYE_Z, EYE_R = 0.046, 0.020, 0.019
FRONT = (0.0, 1.0, 0.0)
RELIEF_LIFT = 0.0012
TOOTH_H = 0.012


# ---------------------------------------------------------------- 2D motifs (front view: x right, z up)

def _bezier(p0, p1, p2, n=40):
    t = np.linspace(0, 1, n)[:, None]
    p0, p1, p2 = (np.asarray(p, float) for p in (p0, p1, p2))
    return (1 - t) ** 2 * p0 + 2 * (1 - t) * t * p1 + t ** 2 * p2


def _spiral(center, r0, start_deg, turns, n=90, decay=0.78):
    """Clockwise curl (ลายขด) — Khon monkey costume convention for fur."""
    t = np.linspace(0, 1, n)
    ang = np.radians(start_deg) - t * turns * 2 * np.pi
    r = r0 * (1 - decay * t)
    return np.column_stack([center[0] + r * np.cos(ang), center[1] + r * np.sin(ang)])


def _curl_from(end, r0, start_deg, turns):
    a = np.radians(start_deg)
    center = (end[0] - r0 * np.cos(a), end[1] - r0 * np.sin(a))
    return _spiral(center, r0, start_deg, turns)


def _brow():
    arc = _bezier((0.014, 0.050), (0.048, 0.094), (0.086, 0.068))
    return np.concatenate([arc, _curl_from(arc[-1], 0.012, 70, 1.4)[1:]]), arc


def _mirror(pts):
    return pts * np.array([-1.0, 1.0])


def _project(bvh, pts2d, lift=RELIEF_LIFT):
    """Drop front-view motif points onto the shell."""
    hits, normals = [], []
    for x, z in pts2d:
        hit, n = geo.ray(bvh, (x, -0.6, z), FRONT)
        if hit is not None:
            hits.append(hit + n * lift)
            normals.append(n)
    return np.array(hits), np.array(normals)


# ---------------------------------------------------------------- eyes

def eye_anchors(bvh):
    anchors = []
    for side in (-1, 1):
        hit, n = geo.ray(bvh, (side * EYE_X, -0.6, EYE_Z), FRONT)
        if hit is None:
            raise RuntimeError("eye ray missed the shell")
        anchors.append({"center": hit, "look": geo.normalize(n + np.array([0.0, -1.2, 0.0]))})
    return anchors


def add_eyes(anchors, mats, root):
    balls, pupils, rims = [], [], []
    for a in anchors:
        rot = geo.frame(a["look"])
        balls.append(geo.transform(rot, a["center"]))
        pupils.append(geo.transform(rot, a["center"] + a["look"] * EYE_R * 0.86, (1, 1, 0.35)))
        rims.append(geo.transform(rot, a["center"]))
    geo.mesh_object("Eyes", *geo.instances(geo.uv_sphere(EYE_R, 40, 20), balls), mats["eye_white"], root)
    geo.mesh_object("Pupils", *geo.instances(geo.uv_sphere(EYE_R * 0.5, 32, 16), pupils),
                    mats["lacquer_black"], root)
    geo.mesh_object("EyeRims", *geo.instances(geo.torus(EYE_R * 1.03, 0.0022, 64, 12), rims),
                    mats["gold_leaf"], root)


# ---------------------------------------------------------------- gold relief + glass inlay

def outline_regions(shell, bvh, mats, root):
    """Gold lines hide the stair-stepped seams between painted regions."""
    chains = geo.material_boundaries(shell, lambda a, b: shape.WHITE in (a, b))
    lines, closed = [], []
    for pts, is_closed in chains:
        if len(pts) < 5:
            continue
        smooth = geo.relax_polyline(pts, 12, is_closed)
        lines.append(geo.snap(bvh, smooth, RELIEF_LIFT))
        closed.append(is_closed)
    geo.tube_object("RegionOutlines", lines, 0.0013, mats["gold_leaf"], root, closed)


def _gems_along(bvh, pts2d, spacing):
    """Evenly spaced cabochon placements along a front-view path."""
    seg = np.linalg.norm(np.diff(pts2d, axis=0), axis=1)
    dist = np.concatenate([[0], np.cumsum(seg)])
    marks = np.arange(spacing * 0.5, dist[-1], spacing)
    picks = np.column_stack([np.interp(marks, dist, pts2d[:, i]) for i in range(2)])
    hits, normals = _project(bvh, picks, lift=0.0008)
    return [(h, n) for h, n in zip(hits, normals)]


def _cabochons(name, placements, radius, material, root):
    mats = [geo.transform(geo.frame(n), h, (1, 1, 0.45)) for h, n in placements]
    if mats:
        geo.mesh_object(name, *geo.instances(geo.uv_sphere(radius, 20, 10), mats), material, root)


def add_brows_and_curls(bvh, mats, root):
    brow, arc = _brow()
    cheek = _spiral((0.072, -0.012), 0.011, 90, 1.6)
    whisker = _spiral((0.052, -0.050), 0.009, 90, 1.4)
    bridge = _bezier((0.010, 0.040), (0.021, 0.015), (0.017, -0.012), 16)

    heavy, light, gems = [], [], []
    for side in (1, -1):
        flip = (lambda p: p) if side > 0 else _mirror
        heavy.append(_project(bvh, flip(brow))[0])
        light += [_project(bvh, flip(m))[0] for m in (cheek, whisker, bridge)]
        gems += _gems_along(bvh, flip(arc), 0.011)
    geo.tube_object("Brows", heavy, 0.0026, mats["gold_leaf"], root)
    geo.tube_object("Curls", [c for c in light if len(c) > 2], 0.0016, mats["gold_leaf"], root)
    _cabochons("BrowInlay", gems, 0.0034, mats["glass_green"], root)

    jewel, n = geo.ray(bvh, (0.0, -0.6, 0.074), FRONT)
    if jewel is not None:
        _cabochons("ForeheadJewel", [(jewel, n)], 0.0062, mats["glass_red"], root)
        ring = geo.transform(geo.frame(n), jewel + n * 0.001)
        geo.mesh_object("ForeheadSetting", *geo.instances(geo.torus(0.0066, 0.0015, 40, 8), [ring]),
                        mats["gold_leaf"], root)


# ---------------------------------------------------------------- mouth

def _root(bvh, x, y, z, up):
    hit, _ = geo.ray(bvh, (x, y, z), (0.0, 0.0, 1.0 if up else -1.0))
    return hit


def _tooth_row(bvh, y_lip, up, angles, sink):
    cz = shape.MOUTH["center"][2]
    placed = []
    for th in np.radians(angles):
        x, y = 0.05 * np.sin(th), y_lip + 0.016 + 0.05 * (1 - np.cos(th))
        root = _root(bvh, x, y, cz, up)
        if root is not None:
            placed.append((root, th))
    sign = -1.0 if up else 1.0
    return [(r + np.array([0, 0, sign * (sink)]), th) for r, th in placed]


def add_mouth(bvh, mats, root):
    lip, _ = geo.ray(bvh, (0.0, -0.6, -0.058), FRONT)
    if lip is None:
        raise RuntimeError("upper lip ray missed the shell")
    y_lip = lip[1]
    tooth = geo.box(0.0075, 0.006, TOOTH_H)
    fang = geo.cone(0.0048, 0.0006, 0.030, 16)
    down = geo.frame((0, 0, -1), (0, -1, 0))

    teeth, fangs = [], []
    for r, th in _tooth_row(bvh, y_lip, True, np.linspace(-44, 44, 6), TOOTH_H / 2 - 0.002):
        teeth.append(geo.transform(geo.frame((0, 0, 1), (np.sin(th), -np.cos(th), 0)), r))
    for r, th in _tooth_row(bvh, y_lip, False, np.linspace(-38, 38, 6), TOOTH_H / 2 - 0.002):
        teeth.append(geo.transform(geo.frame((0, 0, 1), (np.sin(th), -np.cos(th), 0)), r))
    for r, _ in _tooth_row(bvh, y_lip, True, (-62, 62), 0.012):
        fangs.append(geo.transform(down, r))
    for r, _ in _tooth_row(bvh, y_lip, False, (-56, 56), 0.011):
        fangs.append(geo.transform(None, r))

    teeth_obj = geo.mesh_object("Teeth", *geo.instances(tooth, teeth), mats["ivory"], root)
    bevel = teeth_obj.modifiers.new("Round", "BEVEL")
    bevel.width, bevel.segments = 0.0016, 3
    geo.mesh_object("Fangs", *geo.instances(fang, fangs), mats["ivory"], root)
    _crystal_fang(bvh, y_lip, mats, root)
    _tongue(bvh, y_lip, mats, root)


def _crystal_fang(bvh, y_lip, mats, root):
    """เขี้ยวแก้ว — Hanuman's crystal fang, hanging from the centre of the palate."""
    palate = _root(bvh, 0.0, y_lip + 0.04, shape.MOUTH["center"][2], True)
    if palate is None:
        return
    crystal = geo.cone(0.0062, 0.0008, 0.028, 6)
    m = geo.transform(geo.frame((0, 0, -1), (0, -1, 0)), palate - np.array([0, 0, 0.011]))
    geo.mesh_object("CrystalFang", *geo.instances(crystal, [m]), mats["crystal"], root, smooth=False)


def _tongue(bvh, y_lip, mats, root):
    floor = _root(bvh, 0.0, y_lip + 0.07, shape.MOUTH["center"][2], False)
    if floor is None:
        return
    m = geo.transform(None, floor + np.array([0, 0, 0.003]), (0.9, 1.1, 0.3))
    geo.mesh_object("Tongue", *geo.instances(geo.uv_sphere(0.03, 32, 16), [m]), mats["tongue"], root)


# ---------------------------------------------------------------- ears

def add_ear_rings(bvh, mats, root):
    """Kundala (กุณฑล): gold hoop with a teardrop pendant set with a red stone."""
    hoops, drops, stones = [], [], []
    for side in (-1, 1):
        hit, n = geo.ray(bvh, (side * 0.3, 0.03, -0.035), (-side, 0.0, 0.0))
        if hit is None:
            continue
        hang = hit + np.array([side * 0.004, 0.0, -0.012])
        hoops.append(geo.transform(geo.frame((1, 0, 0)), hang))
        drop = hang - np.array([0, 0, 0.022])
        drops.append(geo.transform(None, drop, (1, 1, 1.5)))
        stones.append(geo.transform(geo.frame((side, 0, 0)), drop + np.array([side * 0.0065, 0, 0]),
                                    (1, 1, 0.45)))
    geo.mesh_object("EarHoops", *geo.instances(geo.torus(0.011, 0.0022, 40, 10), hoops),
                    mats["gold_leaf"], root)
    geo.mesh_object("EarDrops", *geo.instances(geo.uv_sphere(0.0075, 24, 12), drops),
                    mats["gold_leaf"], root)
    geo.mesh_object("EarStones", *geo.instances(geo.uv_sphere(0.0042, 20, 10), stones),
                    mats["glass_red"], root)
