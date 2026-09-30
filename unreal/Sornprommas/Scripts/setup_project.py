"""One-time project setup, run headless by the editor:

    UnrealEditor-Cmd Sornprommas.uproject -run=pythonscript -script="<abs path>/Scripts/setup_project.py"

Creates the only binary assets the demo needs:
  /Game/Materials/M_Khon             lit, params BaseColor / Metallic / Roughness / Emissive
  /Game/Materials/M_KhonTranslucent  unlit translucent, params BaseColor / Emissive / Opacity
  /Game/Maps/Battle                  empty level; the game mode builds everything at runtime
"""
import unreal

MAT_DIR = "/Game/Materials"
MAP_PATH = "/Game/Maps/Battle"

tools = unreal.AssetToolsHelpers.get_asset_tools()
lib = unreal.MaterialEditingLibrary
assets = unreal.EditorAssetLibrary


def scalar(mat, name, default, x, y):
    node = lib.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, x, y)
    node.set_editor_property("parameter_name", name)
    node.set_editor_property("default_value", default)
    return node


def make_material(name, translucent):
    path = f"{MAT_DIR}/{name}"
    if assets.does_asset_exist(path):
        assets.delete_asset(path)
    mat = tools.create_asset(name, MAT_DIR, unreal.Material, unreal.MaterialFactoryNew())

    base = lib.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -700, -100)
    base.set_editor_property("parameter_name", "BaseColor")
    base.set_editor_property("default_value", unreal.LinearColor(1, 1, 1, 1))
    emissive = scalar(mat, "Emissive", 0.0, -700, 200)
    glow = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -400, 200)
    lib.connect_material_expressions(base, "", glow, "A")
    lib.connect_material_expressions(emissive, "", glow, "B")

    if translucent:
        mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
        mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
        opacity = scalar(mat, "Opacity", 1.0, -700, 400)
        lit = lib.create_material_expression(mat, unreal.MaterialExpressionAdd, -200, 100)
        lib.connect_material_expressions(base, "", lit, "A")
        lib.connect_material_expressions(glow, "", lit, "B")
        lib.connect_material_property(lit, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        lib.connect_material_property(opacity, "", unreal.MaterialProperty.MP_OPACITY)
    else:
        metallic = scalar(mat, "Metallic", 0.0, -700, 0)
        roughness = scalar(mat, "Roughness", 0.7, -700, 100)
        lib.connect_material_property(base, "", unreal.MaterialProperty.MP_BASE_COLOR)
        lib.connect_material_property(metallic, "", unreal.MaterialProperty.MP_METALLIC)
        lib.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
        lib.connect_material_property(glow, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    lib.recompile_material(mat)
    assets.save_asset(path)
    unreal.log(f"[setup] created {path}")


def make_map():
    if assets.does_asset_exist(MAP_PATH):
        unreal.log(f"[setup] {MAP_PATH} exists")
        return
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not levels.new_level(MAP_PATH):
        unreal.log_error(f"[setup] could not create {MAP_PATH}")
        return
    levels.save_current_level()
    unreal.log(f"[setup] created {MAP_PATH}")


make_material("M_Khon", translucent=False)
make_material("M_KhonTranslucent", translucent=True)
make_map()
