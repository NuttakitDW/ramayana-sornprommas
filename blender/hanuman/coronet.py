"""Gold coronet (มาลัยทอง) circling Hanuman's bald crown.

A band conforming to the dome, pearl rows on both edges, upright กระจัง
petals along the top, and alternating red/green glass set into the band.
"""
import numpy as np

from khon import geometry as geo

CENTER = np.array([0.0, 0.03, 0.125])
TILT_DEG = 12.0          # front sits lower than the back
HEIGHT = 0.022
THICK = 0.004
LIFT = 0.001
SAMPLES = 240
PETALS = 20


def _plane():
    t = np.radians(TILT_DEG)
    u = np.array([1.0, 0.0, 0.0])
    v = np.array([0.0, np.cos(t), np.sin(t)])
    return u, v, np.cross(u, v)


def _ring(bvh, lift_along_axis):
    """Surface points and normals around the head on a plane shifted along its axis."""
    u, v, axis = _plane()
    origin = CENTER + axis * lift_along_axis
    pts, normals = [], []
    for phi in np.linspace(0, 2 * np.pi, SAMPLES, endpoint=False):
        hit, n = geo.ray(bvh, origin, np.cos(phi) * u + np.sin(phi) * v)
        if hit is None:
            raise RuntimeError("coronet ray escaped the shell")
        pts.append(hit)
        normals.append(n)
    return geo.relax_polyline(pts, 3, True), geo.relax_polyline(normals, 3, True)


def _band(lower, lower_n, upper, upper_n):
    rows = [lower + lower_n * LIFT, lower + lower_n * (LIFT + THICK),
            upper + upper_n * (LIFT + THICK), upper + upper_n * LIFT]
    verts = np.concatenate(rows)
    faces = []
    for i in range(SAMPLES):
        j = (i + 1) % SAMPLES
        for r in range(4):
            a, b = r * SAMPLES, ((r + 1) % 4) * SAMPLES
            faces.append([a + i, a + j, b + j, b + i])
    return verts, faces


def _petal_outline(width, height, n=24):
    s = np.linspace(0, 1, n)
    half = width / 2 * np.cos(np.pi * s / 2)
    right = np.column_stack([half, height * s])
    left = np.column_stack([-half[::-1], height * s[::-1]])
    return np.concatenate([right, left[1:-1]])


def add_coronet(bvh, mats, root):
    lower, lower_n = _ring(bvh, -HEIGHT / 2)
    upper, upper_n = _ring(bvh, HEIGHT / 2)
    geo.mesh_object("CoronetBand", *_band(lower, lower_n, upper, upper_n), mats["gold_leaf"], root)

    top = upper + upper_n * (LIFT + THICK * 0.5)
    bottom = lower + lower_n * (LIFT + THICK * 0.5)
    pearls = [geo.transform(None, p) for p in np.concatenate([top[::2], bottom[::2]])]
    geo.mesh_object("CoronetPearls", *geo.instances(geo.uv_sphere(0.0026, 12, 8), pearls),
                    mats["gold_leaf"], root)

    _add_petals(top, upper_n, mats, root)
    _add_band_gems(lower, lower_n, upper, upper_n, mats, root)


def _front_index(pts):
    return int(np.argmin(pts[:, 1]))


def _add_petals(top, normals, mats, root):
    _, _, axis = _plane()
    front = _front_index(top)
    small = geo.slab(_petal_outline(0.018, 0.028), 0.003)
    large = geo.slab(_petal_outline(0.032, 0.05), 0.0035)
    small_m, large_m, gems_red, gems_green = [], [], [], []
    step = SAMPLES // PETALS
    for k in range(PETALS):
        i = (front + k * step) % SAMPLES
        n = normals[i]
        tangent = top[(i + 1) % SAMPLES] - top[i - 1]
        up = geo.normalize(axis + 0.3 * n)
        x = geo.normalize(tangent - np.dot(tangent, up) * up)
        z = np.cross(x, up)
        if np.dot(z, n) < 0:
            x, z = -x, -z
        rot = np.stack([x, up, z], axis=1)
        base = top[i] - up * 0.002
        (large_m if k == 0 else small_m).append(geo.transform(rot, base))
        gem_at = base + up * (0.019 if k == 0 else 0.011) + z * 0.002
        gem = geo.transform(rot, gem_at, (1, 1, 0.45))
        (gems_red if k % 2 == 0 else gems_green).append(gem)
    geo.mesh_object("CoronetPetals", *geo.instances(small, small_m), mats["gold_leaf"], root, smooth=False)
    geo.mesh_object("CoronetCrest", *geo.instances(large, large_m), mats["gold_leaf"], root, smooth=False)
    geo.mesh_object("PetalRubies", *geo.instances(geo.uv_sphere(0.0036, 20, 10), gems_red),
                    mats["glass_red"], root)
    geo.mesh_object("PetalEmeralds", *geo.instances(geo.uv_sphere(0.0032, 20, 10), gems_green),
                    mats["glass_green"], root)


def _add_band_gems(lower, lower_n, upper, upper_n, mats, root):
    mid = (lower + upper) / 2
    mid_n = np.array([geo.normalize(n) for n in (lower_n + upper_n)])
    red, green = [], []
    for k, i in enumerate(range(0, SAMPLES, 10)):
        m = geo.transform(geo.frame(mid_n[i]), mid[i] + mid_n[i] * (LIFT + THICK), (1, 1, 0.5))
        (red if k % 2 == 0 else green).append(m)
    geo.mesh_object("BandRubies", *geo.instances(geo.uv_sphere(0.0042, 20, 10), red), mats["glass_red"], root)
    geo.mesh_object("BandEmeralds", *geo.instances(geo.uv_sphere(0.0042, 20, 10), green),
                    mats["glass_green"], root)
