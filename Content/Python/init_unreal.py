"""
init_unreal.py — Automatically loaded by Unreal Engine PythonScriptPlugin on startup.
Initializes the Agent File Proxy bridge for automated command execution.
"""

import os
import sys

# Add scripts directory to sys.path
PROJECT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
SCRIPTS_DIR = os.path.join(PROJECT_DIR, "scripts")

if SCRIPTS_DIR not in sys.path:
    sys.path.insert(0, SCRIPTS_DIR)

try:
    import editor_file_proxy
    editor_file_proxy.start_file_proxy()
except Exception as e:
    import unreal
    unreal.log_error(f"[init_unreal] Failed to start editor_file_proxy: {e}")
