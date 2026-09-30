"""Turn a signed-distance function into a quad mesh with naive surface nets."""
import numpy as np

# Cell corners indexed i + 2j + 4k, and the 12 cell edges as corner pairs.
_CORNERS = np.array([[i, j, k] for k in (0, 1) for j in (0, 1) for i in (0, 1)], np.float32)
_EDGES = [(0, 1), (2, 3), (4, 5), (6, 7),
          (0, 2), (1, 3), (4, 6), (5, 7),
          (0, 4), (1, 5), (2, 6), (3, 7)]


def sample(fn, lo, hi, spacing):
    """Evaluate fn on a regular grid, one z-slice at a time to bound memory."""
    lo = np.asarray(lo, np.float32)
    axes = [np.arange(lo[i], hi[i] + spacing * 0.5, spacing, dtype=np.float32) for i in range(3)]
    gx, gy = np.meshgrid(axes[0], axes[1], indexing="ij")
    field = np.empty((len(axes[0]), len(axes[1]), len(axes[2])), np.float32)
    for k, z in enumerate(axes[2]):
        pts = np.stack([gx, gy, np.full_like(gx, z)], axis=-1).reshape(-1, 3)
        field[:, :, k] = fn(pts).reshape(gx.shape)
    return field, lo


def _cell_vertices(field):
    """One vertex per sign-changing cell, at the mean of its edge crossings."""
    cx, cy, cz = (n - 1 for n in field.shape)
    acc = np.zeros((cx, cy, cz, 3), np.float32)
    cnt = np.zeros((cx, cy, cz), np.float32)
    for a, b in _EDGES:
        ia, ib = _CORNERS[a].astype(int), _CORNERS[b].astype(int)
        fa = field[ia[0]:ia[0] + cx, ia[1]:ia[1] + cy, ia[2]:ia[2] + cz]
        fb = field[ib[0]:ib[0] + cx, ib[1]:ib[1] + cy, ib[2]:ib[2] + cz]
        cross = (fa < 0) != (fb < 0)
        t = np.where(cross, fa / np.where(cross, fa - fb, 1.0), 0.0)
        for axis in range(3):
            pa, pb = _CORNERS[a][axis], _CORNERS[b][axis]
            acc[..., axis] += np.where(cross, pa + t * (pb - pa), 0.0)
        cnt += cross
    active = cnt > 0
    index = np.full(active.shape, -1, np.int64)
    index[active] = np.arange(int(active.sum()))
    cells = np.argwhere(active).astype(np.float32)
    local = acc[active] / cnt[active][:, None]
    return cells + local, index


def _quads(inside, index):
    """A quad around every grid edge whose endpoints straddle the surface."""
    out = []
    sx = inside[:-1, :, :] != inside[1:, :, :]
    i, j, k = np.nonzero(sx[:, 1:-1, 1:-1])
    j, k = j + 1, k + 1
    q = np.stack([index[i, j - 1, k - 1], index[i, j, k - 1], index[i, j, k], index[i, j - 1, k]], 1)
    out.append(np.where(inside[i + 1, j, k][:, None], q[:, ::-1], q))

    sy = inside[:, :-1, :] != inside[:, 1:, :]
    i, j, k = np.nonzero(sy[1:-1, :, 1:-1])
    i, k = i + 1, k + 1
    q = np.stack([index[i - 1, j, k - 1], index[i - 1, j, k], index[i, j, k], index[i, j, k - 1]], 1)
    out.append(np.where(inside[i, j + 1, k][:, None], q[:, ::-1], q))

    sz = inside[:, :, :-1] != inside[:, :, 1:]
    i, j, k = np.nonzero(sz[1:-1, 1:-1, :])
    i, j = i + 1, j + 1
    q = np.stack([index[i - 1, j - 1, k], index[i, j - 1, k], index[i, j, k], index[i - 1, j, k]], 1)
    out.append(np.where(inside[i, j, k + 1][:, None], q[:, ::-1], q))

    quads = np.concatenate(out)
    if (quads < 0).any():
        raise RuntimeError("surface nets produced a quad on an inactive cell; pad the bounds")
    return quads


def surface_nets(field, origin, spacing):
    """Return (verts, quads). Winding may be mixed; recalculate normals afterwards."""
    grid_verts, index = _cell_vertices(field)
    quads = _quads(field < 0, index)
    return origin + grid_verts * spacing, quads
