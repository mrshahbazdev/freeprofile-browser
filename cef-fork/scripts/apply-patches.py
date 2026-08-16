#!/usr/bin/env python3
"""Apply FreeProfile anti-detect source patches to a Chromium/CEF checkout.

Usage:
  python3 apply-patches.py <chromium/src>

The script is intentionally regex/string-based rather than relying on exact line
numbers so it can be retargeted across Chromium minor versions with minimal
manual work.  It prints every change and warns about any file/function it could
not patch.
"""

import pathlib
import re
import shutil
import sys

SCRIPT_DIR = pathlib.Path(__file__).resolve().parent
REPO_DIR = SCRIPT_DIR.parent
CHROMIUM_SRC = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else pathlib.Path("chromium/src")

FP_INCLUDE = "third_party/blink/renderer/freeprofile/fp_config.h"


def copy_freeprofile_config():
    src = REPO_DIR / "freeprofile"
    dst = CHROMIUM_SRC / "third_party/blink/renderer/freeprofile"
    if dst.exists():
        shutil.rmtree(dst)
    shutil.copytree(src, dst)
    print(f"[OK] copied {src} -> {dst}")


def read(path: pathlib.Path):
    with open(path, "r", encoding="utf-8") as f:
        return f.read()


def write(path: pathlib.Path, text: str):
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)


def add_include(text: str, header: str) -> str:
    if header in text:
        return text
    lines = text.splitlines()
    for i, line in enumerate(lines):
        if line.startswith("#include"):
            lines.insert(i + 1, f'#include "{header}"')
            return "\n".join(lines)
    return f'#include "{header}"\n' + text


def insert_after_signature(text: str, signature: str, code: str, max_matches: int = 1) -> tuple:
    """Insert |code| after the first line of each non-comment function signature.

    Returns (new_text, matched_count).
    """
    lines = text.splitlines()
    inserted = 0
    out = []
    i = 0
    while i < len(lines):
        line = lines[i]
        out.append(line)
        stripped = line.strip()
        if signature in line and not stripped.startswith("//") and not stripped.startswith("/*"):
            # Heuristic: the function body opening brace is on this line.
            indent = "  "
            for codeline in code.strip().splitlines():
                out.append(indent + codeline)
            inserted += 1
            if inserted >= max_matches:
                out.extend(lines[i + 1:])
                break
        i += 1
    return "\n".join(out), inserted


def insert_after_pattern(text: str, pattern: str, code: str, count: int = 0) -> tuple:
    """Insert |code| after each match of |pattern| (regex). count=0 means all."""
    matches = list(re.finditer(pattern, text, re.DOTALL))
    if count:
        matches = matches[:count]
    total = len(matches)
    for m in reversed(matches):
        insert_pos = m.end()
        text = text[:insert_pos] + "\n" + code.strip() + text[insert_pos:]
    return text, total


def patch_file(rel_path: pathlib.Path, patches: dict):
    path = CHROMIUM_SRC / rel_path
    if not path.exists():
        print(f"[WARN] file not found: {path}")
        return
    text = read(path)
    original = text

    if "include" in patches:
        text = add_include(text, patches["include"])

    for signature, code in patches.get("inserts", []):
        text, n = insert_after_signature(text, signature, code, max_matches=patches.get("max_matches", 1))
        if n == 0:
            print(f"[WARN] signature not found in {rel_path}: {signature[:60]}")
        else:
            print(f"[OK] patched {n}x {signature[:60]} in {rel_path}")

    if "regex" in patches:
        text, n = insert_after_pattern(text, patches["regex"]["pattern"], patches["regex"]["code"])
        if n == 0:
            print(f"[WARN] regex not found in {rel_path}")
        else:
            print(f"[OK] patched {n}x regex in {rel_path}")

    if text != original:
        write(path, text)


PATCHES = {
    # ---- Navigator / identity ----
    pathlib.Path("third_party/blink/renderer/core/frame/navigator.cc"): {
        "include": FP_INCLUDE,
        "inserts": [
            ('String Navigator::userAgent() const {',
             'std::string fp_ua = freeprofile::FpSwitch("fp-user-agent");\n'
             'if (!fp_ua.empty()) return String(fp_ua.c_str());'),
            ('String Navigator::platform() const {',
             'std::string fp_plat = freeprofile::FpSwitch("fp-platform");\n'
             'if (!fp_plat.empty()) return String(fp_plat.c_str());'),
            ('String Navigator::vendor() const {',
             'std::string fp_vendor = freeprofile::FpSwitch("fp-vendor");\n'
             'if (!fp_vendor.empty()) return String(fp_vendor.c_str());'),
            ('String Navigator::productSub() const {',
             'std::string fp_sub = freeprofile::FpSwitch("fp-product-sub");\n'
             'if (!fp_sub.empty()) return String(fp_sub.c_str());'),
        ],
    },

    pathlib.Path("third_party/blink/renderer/core/frame/navigator_concurrent_hardware.cc"): {
        "include": FP_INCLUDE,
        "inserts": [
            ('unsigned NavigatorConcurrentHardware::hardwareConcurrency() const {',
             'int fp_cores = freeprofile::FpSwitchInt("fp-hardware-concurrency");\n'
             'if (fp_cores > 0) return static_cast<unsigned>(fp_cores);'),
        ],
    },

    pathlib.Path("third_party/blink/renderer/core/frame/navigator_device_memory.cc"): {
        "include": FP_INCLUDE,
        "inserts": [
            ('float NavigatorDeviceMemory::deviceMemory() const {',
             'int fp_mem = freeprofile::FpSwitchInt("fp-device-memory");\n'
             'if (fp_mem > 0) return static_cast<float>(fp_mem);'),
        ],
    },

    # ---- Screen / window dimensions ----
    pathlib.Path("third_party/blink/renderer/core/frame/screen.cc"): {
        "include": FP_INCLUDE,
        "inserts": [
            ('int Screen::width() const {', 'int fp_w = freeprofile::FpSwitchInt("fp-screen-width");\nif (fp_w > 0) return fp_w;'),
            ('int Screen::height() const {', 'int fp_h = freeprofile::FpSwitchInt("fp-screen-height");\nif (fp_h > 0) return fp_h;'),
            ('int Screen::availWidth() const {', 'int fp_w = freeprofile::FpSwitchInt("fp-screen-width");\nif (fp_w > 0) return fp_w;'),
            ('int Screen::availHeight() const {', 'int fp_h = freeprofile::FpSwitchInt("fp-screen-height");\nif (fp_h > 0) return fp_h;'),
            ('unsigned Screen::colorDepth() const {', 'int fp_d = freeprofile::FpSwitchInt("fp-screen-depth");\nif (fp_d > 0) return static_cast<unsigned>(fp_d);'),
            ('unsigned Screen::pixelDepth() const {', 'int fp_d = freeprofile::FpSwitchInt("fp-screen-depth");\nif (fp_d > 0) return static_cast<unsigned>(fp_d);'),
        ],
    },

    pathlib.Path("third_party/blink/renderer/core/frame/local_dom_window.cc"): {
        "include": FP_INCLUDE,
        "inserts": [
            ('int LocalDOMWindow::innerWidth() const {', 'int fp_w = freeprofile::FpSwitchInt("fp-screen-width");\nif (fp_w > 0) return fp_w;'),
            ('int LocalDOMWindow::innerHeight() const {', 'int fp_h = freeprofile::FpSwitchInt("fp-screen-height");\nif (fp_h > 0) return fp_h;'),
            ('int LocalDOMWindow::outerWidth() const {', 'int fp_w = freeprofile::FpSwitchInt("fp-screen-width");\nif (fp_w > 0) return fp_w;'),
            ('int LocalDOMWindow::outerHeight() const {', 'int fp_h = freeprofile::FpSwitchInt("fp-screen-height");\nif (fp_h > 0) return fp_h;'),
        ],
    },

    # ---- Battery ----
    pathlib.Path("third_party/blink/renderer/modules/battery/battery_manager.cc"): {
        "include": FP_INCLUDE,
        "inserts": [
            ('bool BatteryManager::charging() {', 'if (freeprofile::FpSwitchBool("fp-battery-charging")) return true;\nif (freeprofile::FpSwitchBool("fp-battery-discharging")) return false;'),
            ('double BatteryManager::level() {', 'double fp_lvl = freeprofile::FpSwitchDouble("fp-battery-level");\nif (fp_lvl > 0.0) return fp_lvl;'),
        ],
    },

    # ---- WebRTC disable ----
    pathlib.Path("third_party/blink/renderer/modules/peerconnection/rtc_peer_connection.cc"): {
        "include": FP_INCLUDE,
        "regex": {
            "pattern": r'if \(context->IsContextDestroyed\(\)\) \{\s*exception_state\.ThrowDOMException\([^;]+;\s*return nullptr;\s*\}',
            "code": """
  if (freeprofile::FpSwitchBool("fp-disable-webrtc")) {
    exception_state.ThrowDOMException(
        DOMExceptionCode::kNotSupportedError,
        "WebRTC is disabled.");
    return nullptr;
  }""",
        },
    },

    pathlib.Path("third_party/blink/renderer/modules/mediastream/media_devices.cc"): {
        "include": FP_INCLUDE,
        "inserts": [
            ('ScriptPromise MediaDevices::enumerateDevices(ScriptState* script_state,',
             'if (freeprofile::FpSwitchBool("fp-disable-webrtc")) {\n'
             '    exception_state.ThrowDOMException(DOMExceptionCode::kNotSupportedError, "enumerateDevices is disabled.");\n'
             '    return ScriptPromise();\n'
             '  }'),
        ],
    },

    # ---- WebGL vendor/renderer ----
    pathlib.Path("third_party/blink/renderer/modules/webgl/webgl_rendering_context_base.cc"): {
        "include": FP_INCLUDE,
        "inserts": [
            ('case WebGLDebugRendererInfo::kUnmaskedVendorWebgl:',
             'std::string fp_webgl_vendor = freeprofile::FpSwitch("fp-webgl-vendor");\n'
             'if (!fp_webgl_vendor.empty()) return WebGLAny(script_state, String(fp_webgl_vendor.c_str()));'),
            ('case WebGLDebugRendererInfo::kUnmaskedRendererWebgl:',
             'std::string fp_webgl_renderer = freeprofile::FpSwitch("fp-webgl-renderer");\n'
             'if (!fp_webgl_renderer.empty()) return WebGLAny(script_state, String(fp_webgl_renderer.c_str()));'),
        ],
    },
}


def main():
    if not CHROMIUM_SRC.exists():
        print(f"[FATAL] Chromium source directory not found: {CHROMIUM_SRC}")
        print("Run scripts/fetch-source.sh first.")
        sys.exit(1)

    copy_freeprofile_config()

    for rel, patches in PATCHES.items():
        patch_file(rel, patches)

    print("\nDone.  Now run scripts/build.sh to compile the patched CEF/Chromium.")


if __name__ == "__main__":
    main()
