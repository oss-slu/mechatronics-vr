---
trigger: always_on
---

## Unreal Engine MCP Tools (mcp-unreal)

This project uses **mcp-unreal** for AI-driven Unreal Engine 5.7 development. The MCP server runs as a local Go binary that bridges to two UE plugins:
- **Remote Control API** (port 30010) — built-in UE plugin for property/function access
- **MCPUnreal** (port 8090) — custom C++ editor plugin for advanced editor operations

### Workflow Guidelines

1. **Always check status first**: Call `status` to verify editor and plugin connections before attempting editor operations.
2. **MANDATORY: Verify Code & APIs with Documentation**:
   - Before writing, modifying, or proposing any Unreal Engine C++, Blueprint, material, or shader code, agents **must always** verify the APIs, function signatures, macros, and class hierarchies against Unreal documentation via `mcp-unreal`.
   - Use `lookup_class` (e.g. `lookup_class(class_name="AActor")`) to inspect class inheritance, exposed properties, valid member functions, and usage notes.
   - Use `lookup_docs` (e.g. `lookup_docs(query="...")`) to search UE 5.7 API docs, patterns, and relevant subsystem guidelines. Note: `lookup_class` and `lookup_docs` do not require the editor to be running.
   - Never guess or hallucinate API signatures, parameters, or deprecated functions.
3. **MANDATORY: Verify Commands & MCP Tool Arguments**:
   - Verify that all console commands, build commands, CLI arguments, and MCP tool parameters are strictly accurate and valid before invoking them.
   - When calling MCP tools, adhere strictly to the tool parameter schema and verify expected behavior against `instructions.md` and server documentation.
   - Cross-check engine console commands via docs or tool schemas before execution.
4. **Build-test cycle**: After editing C++ files, call `build_project` to compile, then `run_tests` to verify. Read build errors carefully — use `lookup_docs` to understand UE APIs.
5. **Actor workflow**: Use `get_level_actors` to discover existing actors, `spawn_actor` to create new ones, `move_actor` to position them, and `set_property`/`get_property` to read/write properties.
6. **Blueprint workflow**: Use `blueprint_query` with operation `list` to find Blueprints, then `list_variables`/`list_functions`/`list_nodes` to inspect them. Use `blueprint_modify` to make changes, and always `compile` after modifications.
7. **Material workflow**: Use `material_ops` with `create` to make new materials, `get_parameters` to inspect existing ones, and `set_parameter` or `set_texture` to modify them.
8. **Save your work**: The editor does not auto-save. After making changes, use `level_ops` with `save_level` or remind the user to save.
9. **PIE testing**: Use `pie_control` with operation `start` to begin a Play In Editor session, `status` to verify it started, and `stop` to end it.

### Prerequisites (must be running)
- Unreal Editor must be open with the MechatronicsVR project loaded
- Verify with: `curl http://localhost:30010/remote/info` (Remote Control API) and `curl -X POST http://localhost:8090/api/status` (MCPUnreal plugin)
