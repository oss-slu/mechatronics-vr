mcp-unreal-docs tool

Usage:
  - python .agents/tools/mcp-unreal-docs.py lookup_docs --query "AActor"
  - python .agents/tools/mcp-unreal-docs.py lookup_class --class "AActor"
  - python .agents/tools/mcp-unreal-docs.py fetch_and_cache --url "https://docs.unrealengine.com/..."

Cache location:
  - .agents/docs_cache/<ue_version>/ (e.g. .agents/docs_cache/5.7/)
  - index.json maps slugs to source_url and local_path

Notes:
  - Requires python3 and ideally the packages: requests, beautifulsoup4, markdownify
  - The tool tries to sanitize pages into LLM-friendly markdown and stores them in the cache.
  - Add /.agents/docs_cache/ to .gitignore to avoid committing cached docs.