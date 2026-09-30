"""Khon mask materials: lacquer, gold leaf, mirrored glass inlay and crystal.

Every material is a single Principled BSDF so it survives glTF export to Unreal;
the gold-leaf crinkle bump is render-only and is dropped on export.
"""
import bpy

PALETTE = {
    # name: (base color, metallic, roughness, coat, transmission, ior)
    "lacquer_white": ((0.86, 0.85, 0.80), 0.0, 0.30, 0.5, 0.0, 1.45),
    "lacquer_red": ((0.60, 0.04, 0.03), 0.0, 0.32, 0.4, 0.0, 1.45),
    "mouth_red": ((0.28, 0.012, 0.018), 0.0, 0.45, 0.2, 0.0, 1.45),
    "tongue": ((0.46, 0.05, 0.06), 0.0, 0.40, 0.3, 0.0, 1.45),
    "lacquer_black": ((0.012, 0.012, 0.016), 0.0, 0.25, 0.6, 0.0, 1.45),
    "gold_leaf": ((1.00, 0.72, 0.32), 1.0, 0.22, 0.0, 0.0, 1.45),
    "glass_green": ((0.04, 0.55, 0.24), 0.8, 0.08, 0.3, 0.0, 1.52),
    "glass_red": ((0.72, 0.03, 0.05), 0.8, 0.08, 0.3, 0.0, 1.52),
    "crystal": ((0.93, 0.97, 1.00), 0.0, 0.02, 0.0, 1.0, 1.55),
    "ivory": ((0.92, 0.89, 0.80), 0.0, 0.35, 0.3, 0.0, 1.45),
    "eye_white": ((0.95, 0.93, 0.86), 0.0, 0.20, 0.7, 0.0, 1.45),
}


def _bsdf(mat):
    if mat.node_tree is None:
        mat.use_nodes = True
    return next(n for n in mat.node_tree.nodes if n.type == "BSDF_PRINCIPLED")


def _principled(name, color, metallic, roughness, coat, transmission, ior):
    mat = bpy.data.materials.new(name)
    inputs = _bsdf(mat).inputs
    inputs["Base Color"].default_value = (*color, 1.0)
    inputs["Metallic"].default_value = metallic
    inputs["Roughness"].default_value = roughness
    inputs["Coat Weight"].default_value = coat
    inputs["Transmission Weight"].default_value = transmission
    inputs["IOR"].default_value = ior
    mat.diffuse_color = (*color, 1.0)
    return mat


def _add_leaf_crinkle(mat, scale=320.0, strength=0.25):
    """Hammered gold-leaf texture as a bump on object-space noise."""
    nodes, links = mat.node_tree.nodes, mat.node_tree.links
    coords = nodes.new("ShaderNodeTexCoord")
    noise = nodes.new("ShaderNodeTexNoise")
    noise.inputs["Scale"].default_value = scale
    noise.inputs["Detail"].default_value = 6.0
    bump = nodes.new("ShaderNodeBump")
    bump.inputs["Strength"].default_value = strength
    links.new(coords.outputs["Object"], noise.inputs["Vector"])
    links.new(noise.outputs["Fac"], bump.inputs["Height"])
    links.new(bump.outputs["Normal"], _bsdf(mat).inputs["Normal"])


def khon_palette():
    mats = {name: _principled(name, *spec) for name, spec in PALETTE.items()}
    _add_leaf_crinkle(mats["gold_leaf"])
    return mats
