# Engine-level anti-detect plan for FreeProfile Browser

## Why JS spoofing is detectable

FreeProfile Browser currently injects `fingerprint.js` into every page. This works for basic sites but advanced bot detectors can see:

- Injected script stack / function names
- `navigator` getter overrides (different native code signatures)
- `toString()` of monkey-patched functions
- Frame / worker context where the script did not run
- Timing / `document.documentElement` attribute consistency checks

Real anti-detect products (GoLogin, Multilogin, Camoufox, CloakBrowser) patch the browser **engine itself** (C++/Blink/Firefox), so spoofed values are returned by native getters and are invisible to page-side inspection.

## What real engine-level patching looks like

Studied open-source anti-detect projects:

- **Camoufox** (`daijro/camoufox`): patches Firefox source with ~49 `.patch` files (e.g. `navigator-spoofing.patch`, `screen-spoofing.patch`, `webgl-spoofing.patch`, `audio-context-spoofing.patch`). It injects a `MaskConfig.hpp` C++ config layer and intercepts native getters in `dom/base/nsGlobalWindowInner.cpp`, `dom/base/Element.cpp`, etc. (see `patches/fingerprint-injection.patch`).
- **CloakBrowser** / **ShardBrowser**: provide a **patched Chromium binary**, but the actual C++ patches are proprietary; the repo is only a launcher/wrapper.
- **Qidian Browser**: described as CEF + ungoogled-chromium with C++ patches, but the public repo returned 403/404 and could not be cloned for verification.

For Chromium/CEF the same concept means patching Blink:

| Surface | Chromium/Blink file to patch |
|---|---|
| `navigator.userAgent` / `platform` / `hardwareConcurrency` | `third_party/blink/renderer/core/frame/navigator.cc` |
| `window.screen` / `innerWidth` / `outerWidth` | `third_party/blink/renderer/core/frame/window.cc` / `screen.cc` |
| `Element.getBoundingClientRect` / `getClientRects` | `third_party/blink/renderer/core/dom/element.cc` |
| WebGL `getParameter` | `third_party/blink/renderer/modules/webgl/webgl_rendering_context_base.cc` |
| Canvas 2D `getImageData` / `toDataURL` | `third_party/blink/renderer/modules/canvas/canvas2d/base_rendering_context_2d.cc` |
| `navigator.getBattery` / `connection` | `third_party/blink/renderer/modules/battery/battery_manager.cc` / `net_info.cc` |
| AudioBuffer / AudioContext | `third_party/blink/renderer/modules/webaudio/audio_buffer.cc` |
| Timezone / locale | `third_party/blink/renderer/platform/runtime_enabled_features.json`, ICU usage, or pref patch |
| WebRTC IP handling | `third_party/blink/renderer/modules/peerconnection/` + network stack |

A minimal engine patch would add a `FreeProfileFingerprintConfig` singleton in C++ that is populated from command-line switches / env var, then short-circuit the native getters above.

## Two possible paths

### Option A: Build a custom CEF/Chromium fork (GoLogin/Orbita style)

Steps:

1. Set up a build machine with 200+ GB disk, 32 GB RAM, 16+ cores, Linux.
2. Clone Chromium/CEF source (~30 GB checkout).
3. Add `fp_` switches parsing in `content/browser` / `content/child`.
4. Patch Blink files (list above).
5. Build `chrome_sandbox` + `cefclient` / `cefsimple` (many hours).
6. Ship custom `chrome` binary + resources with FreeProfile Launcher.

Pros:
- Maximum stealth; passes CreepJS, FingerprintJS, BrowserScan.
- Full control over every fingerprint surface.

Cons:
- 6-24 hours per build.
- Needs ongoing rebase for every Chromium release.
- 100+ GB build artifacts.
- Not feasible inside the current Devin VM.

Estimated timeline: 2-4 engineer-weeks for first working build; ongoing maintenance per Chromium release.

### Option B: Integrate an existing engine-level browser

Use **Camoufox** (open-source patched Firefox) or **CloakBrowser** (patched Chromium wrapper) as the engine and wrap it with the FreeProfile profile manager.

Camoufox integration:

- `pip install camoufox` downloads a pre-built anti-detect Firefox.
- FreeProfile dashboard generates `camoufox` launch config from each profile (proxy, user-agent, screen, timezone, etc.).
- Python launcher (`freeprofile-camoufox-launcher.py`) starts a Camoufox instance per profile.
- Playwright/Juggler provides remote control and automation.

Pros:
- Engine-level spoofing immediately.
- Works today; no 100 GB build.
- Active open-source community (Camoufox).

Cons:
- Not CEF/Chromium-based (if that matters for branding).
- License and dependency on upstream binaries.

## Recommendation

For a shipping product in the shortest time, use **Option B** (Camoufox or CloakBrowser engine) behind the existing FreeProfile profile manager, while keeping the CEF path for users who want a lightweight browser.

For a true GoLogin competitor, plan **Option A** as a second phase and keep the CEF/Chromium fork in a separate repository because of build size.

## Next decision needed

1. Integrate **Camoufox** (patched Firefox) as an engine option in the dashboard.
2. Integrate **CloakBrowser** (patched Chromium wrapper) if license allows.
3. Start a **custom CEF/Chromium fork** build (requires dedicated build server).
