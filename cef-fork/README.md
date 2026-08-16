# FreeProfile CEF/Chromium Fork

Engine-level anti-detect patches for the Chromium/CEF build used by FreeProfile Browser.

## Why this fork?

FreeProfile Browser currently injects JavaScript (`fingerprint.js`) into every page to spoof `navigator`, `screen`, `WebGL`, etc.  Advanced bot detectors can spot injected JS by:

- `toString()` / native code signature checks
- Cross-frame / worker context leaks
- `PerformanceObserver` / timing fingerprinting
- DevTools introspection

This fork moves the spoofing **into the C++ engine** so sites see native getters returning the spoofed values, just like GoLogin/Orbita, Camoufox, or CloakBrowser do.

## What is patched

Current patches live in `scripts/apply-patches.py` and touch:

- `third_party/blink/renderer/core/frame/navigator.cc` — `userAgent`, `platform`, `vendor`, `productSub`
- `third_party/blink/renderer/core/frame/navigator_concurrent_hardware.cc` — `hardwareConcurrency`
- `third_party/blink/renderer/core/frame/navigator_device_memory.cc` — `deviceMemory`
- `third_party/blink/renderer/core/frame/screen.cc` — `width`, `height`, `availWidth`, `availHeight`, `colorDepth`, `pixelDepth`
- `third_party/blink/renderer/core/frame/local_dom_window.cc` — `innerWidth/Height`, `outerWidth/Height`
- `third_party/blink/renderer/modules/battery/battery_manager.cc` — `charging`, `level`
- `third_party/blink/renderer/modules/peerconnection/rtc_peer_connection.cc` — block WebRTC creation when `--fp-disable-webrtc` is set
- `third_party/blink/renderer/modules/mediastream/media_devices.cc` — block `enumerateDevices` when WebRTC disabled
- `third_party/blink/renderer/modules/webgl/webgl_rendering_context_base.cc` — override `UNMASKED_VENDOR_WEBGL` / `UNMASKED_RENDERER_WEBGL`

`freeprofile/fp_config.h` is a header-only helper that reads the same `--fp-*` command-line switches already used by FreeProfile Browser's CEF launcher.

## Build requirements

- Linux (Ubuntu 22.04 recommended) or Windows with WSL2
- 200+ GB free disk
- 32+ GB RAM
- 16+ CPU cores
- Fast internet connection

## Quick start

```bash
# 1. Install system dependencies
bash scripts/bootstrap.sh

# 2. Download CEF + Chromium source (~30-50 GB, can take hours)
bash scripts/fetch-source.sh

# 3. Apply FreeProfile patches and build Release CEF
cd scripts
bash build.sh
```

The build output will be under `work/chromium_git/chromium/src/out/Release_GN_x64/`.

## Docker path

```bash
docker build -t freeprofile-cef .
docker run -v "$PWD/work:/cef-build" --rm -it freeprofile-cef bash scripts/build.sh
```

## Still TODO for full GoLogin parity

- Canvas 2D noise in `base_rendering_context_2d.cc` (`getImageData`, `toDataURL`)
- WebAudio buffer noise in `audio_buffer.cc`
- Font list spoofing / `document.fonts` native return
- ClientRects / DOMRect noise in `element.cc`
- WebRTC IP leak protection at the network layer (`P2PTransportChannel`)
- Timezone / ICU locale override
- `Sec-CH-UA` and TLS ClientHello spoofing
- Coherent profile bundles (matching GPU + screen + OS + UA)

Each surface follows the same pattern: read `--fp-<name>` from `freeprofile::FpSwitch` and short-circuit the native getter.

## License

Patches and helper code are MIT. Chromium/CEF are governed by their own licenses.
