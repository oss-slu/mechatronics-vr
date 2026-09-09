#!/usr/bin/env python3
"""
mcp-unreal-docs.py

Lightweight MCP helper: detect UE version from .uproject, query Unreal docs, sanitize HTML to LLM-friendly Markdown,
cache sanitized pages under .agents/docs_cache/<version>/, and expose JSON CLI commands.

Commands (CLI):
  - lookup_docs --query "text"
  - lookup_class --class "AActor"
  - fetch_and_cache --url "https://docs.unrealengine.com/..."

This script is intentionally small and dependency-light. It uses requests + beautifulsoup4 + markdownify when available,
with fallbacks to simple heuristics.

Note: Run from repo root. Cache location: .agents/docs_cache/<version>/
"""
import argparse
import json
import os
import re
import sys
from pathlib import Path

# Minimal optional imports
try:
    import requests
    from bs4 import BeautifulSoup
    from markdownify import markdownify as mdify
except Exception:
    requests = None
    BeautifulSoup = None
    mdify = None

REPO_ROOT = Path(__file__).resolve().parents[2]
CACHE_ROOT = REPO_ROOT / '.agents' / 'docs_cache'
UPROJECT = REPO_ROOT / 'MechatronicsVR.uproject'


def detect_engine_version():
    if not UPROJECT.exists():
        return 'unknown'
    try:
        data = json.loads(UPROJECT.read_text(encoding='utf-8'))
        engine = data.get('EngineVersion') or data.get('EngineAssociation') or data.get('Engine')
        if isinstance(engine, str):
            # common formats: "5.7" or "++UE5+Release-5.7"
            m = re.search(r'5\.\d+', engine)
            if m:
                return m.group(0)
            return engine
    except Exception:
        pass
    return 'unknown'


def ensure_cache_dir(version):
    p = CACHE_ROOT / version
    p.mkdir(parents=True, exist_ok=True)
    return p


def sanitize_html_to_markdown(html, base_url=None):
    """Sanitize HTML: remove scripts/styles/navigation and convert to plain markdown.
    Falls back to heuristic extraction if markdownify not present.
    """
    if BeautifulSoup is None:
        # Very small heuristic: strip tags
        text = re.sub(r'<(script|style)[\s\S]*?<\\1>', '', html, flags=re.I)
        text = re.sub(r'<[^>]+>', '', text)
        return text

    soup = BeautifulSoup(html, 'html.parser')

    # Remove scripts, styles, nav, header, footer, aside, noscript
    for sel in soup(['script', 'style', 'nav', 'header', 'footer', 'aside', 'noscript', 'form']):
        sel.decompose()

    # Remove common site chrome by id/class patterns (best-effort)
    for bad in soup.select('[class*=nav], [class*=header], [class*=footer], [id*=nav], [id*=header], [id*=footer], [class*=sidebar]'):
        try:
            bad.decompose()
        except Exception:
            pass

    # Inline images that are purely decorative should be removed
    for img in soup.find_all('img'):
        alt = (img.get('alt') or '').lower()
        src = img.get('src') or ''
        if not alt or 'logo' in alt or src.endswith('.svg'):
            img.decompose()

    cleaned_html = str(soup)

    if mdify is not None:
        try:
            md = mdify(cleaned_html, heading_style='ATX')
            # Trim large whitespace
            md = re.sub(r"\n{3,}", '\n\n', md)
            return md.strip()
        except Exception:
            pass

    # Fallback: extract text blocks and preserve code blocks
    # Pull out <pre> and <code> blocks
    code_blocks = []
    for pre in soup.find_all('pre'):
        code = pre.get_text()
        code_blocks.append(code)
        pre.replace_with(soup.new_string('\n\n```\n' + code + '\n```\n\n'))

    text = soup.get_text('\n')
    text = re.sub(r"\n{3,}", '\n\n', text)
    return text.strip()


def fetch_url(url):
    if requests is None:
        raise RuntimeError('requests is required to fetch docs. Install python requests and try again.')
    r = requests.get(url, timeout=15)
    r.raise_for_status()
    return r.text


def fetch_and_cache(url, version):
    cache_dir = ensure_cache_dir(version)
    # produce slug from url
    slug = re.sub(r'[^a-z0-9]+', '-', url.lower()).strip('-')[:120]
    md_path = cache_dir / f'{slug}.md'
    meta_path = cache_dir / 'index.json'

    if md_path.exists():
        return {'cached': True, 'local_path': str(md_path)}

    html = fetch_url(url)
    md = sanitize_html_to_markdown(html, base_url=url)
    md_path.write_text(md, encoding='utf-8')

    # update index
    index = {}
    if meta_path.exists():
        try:
            index = json.loads(meta_path.read_text(encoding='utf-8'))
        except Exception:
            index = {}
    index[slug] = {'source_url': url, 'local_path': str(md_path)}
    meta_path.write_text(json.dumps(index, indent=2), encoding='utf-8')
    return {'cached': False, 'local_path': str(md_path)}


def lookup_docs(query, version, limit=5):
    # Minimal search: query epic docs search endpoint if available, otherwise use site:docs.unrealengine.com Google-like search via duckduckgo html
    # For now, perform a DuckDuckGo site search using ddg and extract first hits.
    hits = []
    if requests is None:
        raise RuntimeError('requests is required to lookup docs. Install python requests and try again.')

    ddg = f'https://duckduckgo.com/html/?q=site:docs.unrealengine.com+{requests.utils.requote_uri(query)}'
    r = requests.get(ddg, timeout=15, headers={'User-Agent': 'mcp-unreal-docs/1.0'})
    r.raise_for_status()
    html = r.text
    if BeautifulSoup is None:
        return []
    soup = BeautifulSoup(html, 'html.parser')
    for a in soup.select('.result__a')[:limit]:
        href = a.get('href')
        title = a.get_text().strip()
        # DuckDuckGo returns /l/?kh=1&uddg=<encoded_url>
        m = re.search(r'uddg=(.+)$', href)
        if m:
            import urllib.parse as up
            real = up.unquote(m.group(1))
        else:
            real = href
        hits.append({'title': title, 'url': real})
    # Fetch and cache first hit(s)
    out = []
    for h in hits:
        try:
            res = fetch_and_cache(h['url'], version)
            out.append({'title': h['title'], 'url': h['url'], 'local_path': res['local_path']})
        except Exception as e:
            out.append({'title': h['title'], 'url': h['url'], 'error': str(e)})
    return out


def main():
    p = argparse.ArgumentParser()
    sub = p.add_subparsers(dest='cmd')

    a_lookup = sub.add_parser('lookup_docs')
    a_lookup.add_argument('--query', required=True)
    a_lookup.add_argument('--version', default=None)

    a_fetch = sub.add_parser('fetch_and_cache')
    a_fetch.add_argument('--url', required=True)
    a_fetch.add_argument('--version', default=None)

    a_class = sub.add_parser('lookup_class')
    a_class.add_argument('--class', dest='classname', required=True)
    a_class.add_argument('--version', default=None)

    args = p.parse_args()
    version = args.version or detect_engine_version()

    if args.cmd == 'lookup_docs':
        res = lookup_docs(args.query, version)
        print(json.dumps(res, indent=2))
        return
    if args.cmd == 'fetch_and_cache':
        res = fetch_and_cache(args.url, version)
        print(json.dumps(res, indent=2))
        return
    if args.cmd == 'lookup_class':
        # naive: search for "AActor" docs
        res = lookup_docs(args.classname, version, limit=3)
        print(json.dumps(res, indent=2))
        return

    p.print_help()


if __name__ == '__main__':
    main()
