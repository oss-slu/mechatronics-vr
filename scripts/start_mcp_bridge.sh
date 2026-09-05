#!/usr/bin/env bash
# start_mcp_bridge.sh — Starts socat TCP port forwarders from WSL to Windows host
# for Unreal Engine Remote Control API (30010) and MCPUnreal Plugin (8090).

pkill -f "socat TCP-LISTEN:30010" 2>/dev/null || true
pkill -f "socat TCP-LISTEN:8090" 2>/dev/null || true

HOST_IP=$(ip route show default | awk '{print $3}')

if [ -z "$HOST_IP" ]; then
    echo "Error: Could not resolve Windows host gateway IP from WSL."
    exit 1
fi

echo "Connecting to Windows Host IP: $HOST_IP"

socat TCP-LISTEN:30010,fork,reuseaddr TCP:${HOST_IP}:30010 >/dev/null 2>&1 &
socat TCP-LISTEN:8090,fork,reuseaddr TCP:${HOST_IP}:8090 >/dev/null 2>&1 &

sleep 1

RC_STATUS=$(curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:30010/remote/info || echo "000")
PLUGIN_STATUS=$(curl -s -o /dev/null -w "%{http_code}" -H "Content-Type: application/json" -d '{}' http://127.0.0.1:8090/api/status || echo "000")

echo "Remote Control API (Port 30010) Status: HTTP $RC_STATUS"
echo "MCPUnreal Plugin   (Port 8090)  Status: HTTP $PLUGIN_STATUS"

if [ "$RC_STATUS" = "200" ] && [ "$PLUGIN_STATUS" = "200" ]; then
    echo "Unreal MCP bridge successfully established!"
else
    echo "Warning: One or both ports did not return 200. Ensure Unreal Editor is open and plugins are loaded."
fi
