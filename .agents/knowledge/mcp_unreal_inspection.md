# Inspecting Blueprints and Levels via the MCPUnreal HTTP API

## Calling the Plugin Directly When mcp-unreal Tools Aren't Loaded

The MCPUnreal plugin (port 8090) can be called with plain `curl -X POST -H "Content-Type: application/json"`. Route names live in `Plugins/MCPUnreal/Source/MCPUnreal/Private/*Routes.cpp` (grep for `"/api/`). Useful read-only ones:
- `/api/blueprints/inspect` `{"blueprint_path": "/Game/.../BP_X"}` -> variables, function graphs, event graphs.
- `/api/blueprints/get_graph` `{"blueprint_path": ..., "graph_name": "EventGraph"}` -> nodes with `pins[].links[]` (`node_id`, `pin_name`).
- `/api/actors/list` `{}` -> JSON array of level actors (not an object).

## /api/actors/components Only Returns the Root Component

It does not show the full component tree. To see every component (including ChildActorComponents and what they spawned), run editor Python through `/api/editor/execute_script` `{"script": "..."}`.

**Gotcha:** `execute_script` returns `"output": ""` — `print`/`unreal.log` output goes to the Output Log. Prefix your log lines (e.g. `unreal.log("XX ...")`) and read them back with `/api/editor/output_log` `{"max_lines": 300, "pattern": "XX "}`. Wrap each query in try/except, because one bad property name aborts the rest of the script silently.

Use `unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()` (the `EditorLevelLibrary` versions are deprecated in 5.7).

## Components Under a ChildActorComponent Belong to the Parent Actor

In a Blueprint, components you attach *under* a ChildActorComponent in the component tree still belong to the parent Blueprint's actor, not to the spawned child actor. Example: in `BP_MotorAssembly`, `Contact1`, `SnapPoint` and `Grab1` sit under `ContactPart` (a ChildActorComponent spawning `APartActor`), but the spawned `PartActor` only has its own default `Mesh`/`GrabComponent`/etc. — no snap point. The part/snap system works per `APartActor`, so a snap point meant for a part must be added inside that part's own Blueprint class.

## Editor Python Property Names Don't Always Match the C++ Name

UE's Python snake_cases names by splitting before every capital and drops the `b` on bools:
- `CompatibleSnapIDs` -> `compatible_snap_i_ds` (not `compatible_snap_ids`)
- `bRequireAllTargetsPresent` -> `require_all_targets_present`; `bAutoStartLesson` -> `auto_start_lesson`

When `get/set_editor_property` fails, list candidates with `[p for p in dir(unreal.SomeClass) if 'part' in p]`.

## Editing Blueprint Components from Python (SubobjectDataSubsystem)

- `sds.k2_gather_subobject_data_for_blueprint(bp)` returns **each component twice**. Dedupe by object name before acting on handles.
- Passing duplicate handles to `delete_subobjects` triggers a handled ensure (`ParentNode`, SimpleConstructionScript.cpp) on the second copy. The delete still works, but dedupe to avoid it.
- Set CDO properties (`unreal.get_default_object(bp.generated_class())`) **after** `compile_blueprint`, then `EditorAssetLibrary.save_asset`. Component template edits made before compiling did survive.
- New component under a parent: `sds.add_new_subobject(unreal.AddNewSubobjectParams(parent_handle=..., new_class=..., blueprint_context=bp))`, then `sds.rename_subobject(handle, unreal.Text("Name"))`.
- Lesson steps in a data asset are instanced: create them with `unreal.new_object(unreal.AssembleStep, outer=data_asset)` so they serialize inside the asset.

## Pasting Blueprint Nodes When the API Can't Create Them

MCP `add_node` only creates bare node classes (fine for `K2Node_IfThenElse`), and Python can't set a node's `VariableReference`/`FunctionReference`. Workaround: put T3D node text on the clipboard (`Get-Content -Raw f.t3d | Set-Clipboard`), have the user Ctrl+V in the graph, then wire with MCP `connect_pins`. Pins can be omitted from the T3D — pasted nodes rebuild them from the reference:

```
Begin Object Class=/Script/BlueprintGraph.K2Node_VariableGet Name="K2Node_VariableGet_X"
   VariableReference=(MemberName="Contact",bSelfContext=True)
   NodePosX=-320
   NodePosY=640
End Object
Begin Object Class=/Script/BlueprintGraph.K2Node_CallFunction Name="K2Node_CallFunction_X"
   FunctionReference=(MemberParent=/Script/CoreUObject.Class'"/Script/MechatronicsVR.AssemblyActor"',MemberName="ArePartsConnected")
End Object
```

`disconnect_pins` takes `node_id` + `pin_name` and breaks **all** links on that pin. Computer-use can't target Unreal Editor (not in its app list), so the paste needs the user.

## Placed Actors Can Hold Stale Overrides of Blueprint Defaults

After spawning an instance and recompiling its Blueprint, the level instance of `BP_ContactPart` had `AssemblyActorClass = None` while the CDO had the right value. Always check the **level instance** (`actor.get_editor_property(...)`) as well as the CDO when a default seems ignored at runtime.

## Can't Set a CableComponent's AttachEndTo.OtherActor from Python

`set_editor_property('other_actor', ...)` on a `ComponentReference` struct fails with "cannot be edited on templates" (edit-flag check), even on a level instance. `component_property` (the component name) can be set. To point a new cable at another component on the same actor, **duplicate an actor whose cable already targets that actor** (`EditorActorSubsystem.duplicate_actor`) and change only `component_property`, then write the struct back with `cab.set_editor_property('attach_end_to', ref)`.

## Adding Object-Reference Blueprint Variables

MCP `add_variable` only sets `PinCategory`, so it can't make a variable of type "BP_X object reference". Use editor Python instead:

```python
bel = unreal.BlueprintEditorLibrary
bel.add_member_variable(bp, 'Contact2', bel.get_object_reference_type(other_bp.generated_class()))
bel.set_blueprint_variable_instance_editable(bp, 'Contact2', True)
bel.compile_blueprint(bp)
```

Placed actors get the new property after compile; set it with `actor.set_editor_property('Contact2', other_actor)` (the C++-style name works for BP variables) and save the level. Compiling a BP re-dirties levels that contain its instances, so save the level again afterward. Pasted T3D nodes land at the mouse cursor, not their `NodePosX/Y`.
