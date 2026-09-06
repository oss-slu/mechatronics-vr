# Unreal Engine MCP (mcp-unreal) Setup Guide

This guide documents the setup, architecture, and network configuration required to run **mcp-unreal** with Unreal Engine 5.7 in this development environment (WSL2 + Windows Host).

---

## Architecture Overview

`mcp-unreal` enables AI agent control over Unreal Engine through two communication bridges:

```
+------------------------------------+          +------------------------------------+
|            WSL2 (Linux)            |          |           Windows Host             |
|                                    |          |                                    |
|   Antigravity Agent (JSON-RPC)     |          |                                    |
|                |                   |          |                                    |
|                v                   |          |                                    |
|      mcp-unreal Go binary          |          |                                    |
|         (Port 30010) (Port 8090)   |          |                                    |
|                |           |       |          |                                    |
|                v           v       |          |                                    |
|        socat Port Forwarders       |  Bridge  |        Unreal Editor 5.7           |
|         :30010      :8090  --------+--------->|  :30010 Remote Control API         |
|                                    |          |  :8090  MCPUnreal Editor Plugin    |
+------------------------------------+          +------------------------------------+
```

1. **Remote Control API (Port 30010)**:
   - Built-in Unreal Engine plugin (`RemoteControl`).
   - Handles object property read/write, function calls, and console commands via REST.
2. **MCPUnreal Plugin (Port 8090)**:
   - Custom C++ Editor plugin (`Plugins/MCPUnreal`).
   - Exposes editor automation: actor hierarchy inspection, Blueprint modification, PIE control, viewport capture, materials, and diagnostics.
3. **mcp-unreal Go Server**:
   - The MCP bridge executable located at `~/gopath/bin/mcp-unreal`.

---

## 1. Unreal Engine Project Setup (Windows)

### A. Enable Required Engine Plugins
In your `.uproject` or via Unreal Editor (**Edit -> Plugins**):
- **Remote Control API** (`RemoteControlWebInterface` / `RemoteControl`)
- **Python Editor Script Plugin** (optional, recommended for script execution)

### B. Install MCPUnreal Plugin
Ensure the `MCPUnreal` plugin is located in:
```
[ProjectRoot]/Plugins/MCPUnreal/
```
In your `MechatronicsVR.uproject`:
```json
{
  "Plugins": [
    {
      "Name": "MCPUnreal",
      "Enabled": true
    }
  ]
}
```

### C. Verify Ports on Windows
When Unreal Editor is open with your project:
- Port `30010` is opened by the Remote Control API.
- Port `8090` is opened by the `MCPUnreal` HTTP server.

You can verify from PowerShell:
```powershell
Test-NetConnection -ComputerName 127.0.0.1 -Port 30010
Test-NetConnection -ComputerName 127.0.0.1 -Port 8090
```

---

## 2. WSL2 Network Bridge Setup

In WSL2, `127.0.0.1` refers to the Linux environment, while Unreal Editor runs on the Windows host. To allow `mcp-unreal` to communicate with the editor, port forwarders route traffic to the Windows gateway.

### A. Install socat (WSL)
```bash
sudo apt update && sudo apt install -y socat
```

### B. Start Port Forwarding
Run the following commands in WSL (or run them via background tasks):
```bash
HOST_IP=$(ip route show default | awk '{print $3}')

# Forward Remote Control API (30010)
socat TCP-LISTEN:30010,fork,reuseaddr TCP:${HOST_IP}:30010 2>/dev/null &

# Forward MCPUnreal Plugin (8090)
socat TCP-LISTEN:8090,fork,reuseaddr TCP:${HOST_IP}:8090 2>/dev/null &
```

> **Tip**: You can use the helper script located at `scripts/start_mcp_bridge.sh`.

---

## 3. mcp-unreal Server Configuration

### A. Build / Install Go Binary
If building from source:
```bash
go install github.com/remiphilippe/mcp-unreal/cmd/mcp-unreal@latest
```
Binary path: `~/gopath/bin/mcp-unreal`

### B. MCP Config Configuration
Configured in [.agents/mcp_config.json](file:///mnt/d/Capstone/mechatronics-vr/.agents/mcp_config.json) or your MCP client settings:
```json
{
  "mcpServers": {
    "mcp-unreal": {
      "command": "/home/mudkipindisguise/gopath/bin/mcp-unreal",
      "env": {
        "MCP_UNREAL_PROJECT": "D:/Capstone/mechatronics-vr/MechatronicsVR.uproject"
      }
    }
  }
}
```

---

## 4. Health Check & Verification

Run these commands in WSL to verify the connection:

```bash
# 1. Check Remote Control API
curl -s http://127.0.0.1:30010/remote/info | head -n 5

# 2. Check MCPUnreal Plugin
curl -s -H "Content-Type: application/json" -d '{}' http://127.0.0.1:8090/api/status
```

Expected responses:
- `30010`: JSON describing available HTTP routes (`/remote/...`).
- `8090`: JSON confirming `"name": "MCPUnreal"`, `"version": "0.2.0"`, `"ue_version": "++UE5+Release-5.7..."`.

### Inside Antigravity / MCP Client:
Call the `status` tool:
```json
{
  "editor_online": true,
  "plugin_online": true,
  "features": ["rc_api_properties", "actors", "blueprints", ...]
}
```
All capabilities are then ready for agent operations.
