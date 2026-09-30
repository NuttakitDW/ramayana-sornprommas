"""Vectorised signed-distance primitives. Points are (N, 3) float32; negative is inside."""
import numpy as np


def rotation(x_deg=0.0, y_deg=0.0, z_deg=0.0):
    """Local-to-world rotation applying X, then Y, then Z."""
    ax, ay, az = np.radians([x_deg, y_deg, z_deg])
    rx = np.array([[1, 0, 0], [0, np.cos(ax), -np.sin(ax)], [0, np.sin(ax), np.cos(ax)]])
    ry = np.array([[np.cos(ay), 0, np.sin(ay)], [0, 1, 0], [-np.sin(ay), 0, np.cos(ay)]])
    rz = np.array([[np.cos(az), -np.sin(az), 0], [np.sin(az), np.cos(az), 0], [0, 0, 1]])
    return (rz @ ry @ rx).astype(np.float32)


def mirror_x(p):
    """Fold space across the YZ plane so one primitive serves both sides of the face."""
    return np.concatenate([np.abs(p[:, :1]), p[:, 1:]], axis=1)


def ellipsoid(p, center, radii, rot=None):
    """Bound-corrected ellipsoid distance (Quilez); exact enough for blending."""
    q = p - np.asarray(center, np.float32)
    if rot is not None:
        q = q @ rot
    r = np.asarray(radii, np.float32)
    k0 = np.linalg.norm(q / r, axis=1)
    k1 = np.linalg.norm(q / (r * r), axis=1)
    return k0 * (k0 - 1.0) / np.maximum(k1, 1e-9)


def above(p, z):
    """Half-space keeping everything above height z."""
    return z - p[:, 2]


def smooth_union(a, b, k):
    h = np.clip(0.5 + 0.5 * (b - a) / k, 0.0, 1.0)
    return b + (a - b) * h - k * h * (1.0 - h)


def smooth_subtract(a, b, k):
    """Carve b out of a with a rounded seam of width k."""
    h = np.clip(0.5 - 0.5 * (a + b) / k, 0.0, 1.0)
    return a + (-b - a) * h + k * h * (1.0 - h)


def intersect(a, b):
    return np.maximum(a, b)
