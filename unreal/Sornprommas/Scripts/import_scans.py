"""Import scanned Khon masks exported by blender/import_hanuman_scan.py:

    UnrealEditor Sornprommas.uproject -run=pythonscript -script="<abs path>/Scripts/import_scans.py"

Nanite stays off: the Interchange glTF materials lack the Nanite usage flag,
and one ~270k-triangle mask renders fine without it.
"""
import os

import unreal

PROJECT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REPO = os.path.dirname(os.path.dirname(PROJECT))
SCANS = [
    (os.path.join(REPO, "blender/build/scans/SM_HanumanMask.glb"), "/Game/Scans/Hanuman"),
]
assets = unreal.EditorAssetLibrary


def import_glb(path, destination):
    task = unreal.AssetImportTask()
    task.filename = path
    task.destination_path = destination
    task.automated = True
    task.replace_existing = True
    task.save = True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths)


def disable_nanite(mesh):
    settings = mesh.get_editor_property("nanite_settings")
    if settings.get_editor_property("enabled"):
        settings.set_editor_property("enabled", False)
        mesh.set_editor_property("nanite_settings", settings)
        assets.save_loaded_asset(mesh)


def meshes_under(destination):
    for path in assets.list_assets(destination, recursive=True):
        asset = unreal.load_asset(path)
        if isinstance(asset, unreal.StaticMesh):
            yield asset


for path, destination in SCANS:
    if not os.path.exists(path):
        unreal.log_error(f"[scans] missing {path}; run the Blender export first")
        continue
    if assets.does_directory_exist(destination):
        assets.delete_directory(destination)  # clean import; reimport keeps stale materials
    import_glb(path, destination)
    for mesh in meshes_under(destination):
        disable_nanite(mesh)
        box = mesh.get_bounding_box()
        unreal.log(f"[scans] {mesh.get_path_name()} bounds min={box.min} max={box.max}")
