# FreeProfile Browser (CEF-based)

A minimal CEF (Chromium Embedded Framework) browser with per-profile isolation, proxy, user-agent support, and a built-in login / profile-manager UI. This is the starting scaffold for a GoLogin-style anti-detect browser.

## Features

- Built-in login screen + profile manager UI (`ui/`).
- Per-profile isolated cache / cookies / storage (each process uses its own `cache_path`).
- Proxy support per profile (`http://user:pass@host:port` or `socks5://...`).
- Custom user-agent per profile.
- Launch any starting URL per profile.
- Cross-platform CEF Views UI (works on Linux, macOS, Windows).
- Off-screen rendering (`--windowless`) for headless / automation use.

## Project structure

- `main.cpp` / `main_win.cpp` — platform entry points.
- `app.h` / `app.cc` — `CefApp` / `CefBrowserProcessHandler` implementation.
- `handler.h` / `handler.cc` — `CefClient`, display / life-span / request handlers.
- `message_handler.h` / `message_handler.cc` — CEF message-router handler for JS-to-C++ queries.
- `profile_manager.h` / `profile_manager.cc` — profile persistence (JSON on disk).
- `render_handler.h` / `render_handler.cc` — off-screen render handler.
- `render_process_handler.h` / `render_process_handler.cc` — renderer-side message router setup.
- `ui/` — HTML/CSS/JS login and profile manager interface.
- `CMakeLists.txt` — build configuration.

## Build requirements

- CMake 3.21+
- C++20 compiler (GCC 11+, Visual Studio 2022, Xcode)
- CEF binary distribution matching your platform: https://cef-builds.spotifycdn.com/index.json
- Linux: `build-essential libx11-dev` (also GTK/DBus runtime libs).

## Build (Linux / macOS)

```bash
export CEF_ROOT=/path/to/cef_binary_..._linux64
mkdir build && cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

## Build (Windows)

```cmd
set CEF_ROOT=C:\path\to\cef_binary_..._windows64
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
```

## Run

Linux (opens the login UI by default):
```bash
./build/Release/freeprofile-browser
```

With a profile, proxy, and custom UA (bypasses the login UI):
```bash
./build/Release/freeprofile-browser \
  --profile-dir=/tmp/fp-profile-work \
  --proxy=http://user:pass@1.2.3.4:8080 \
  --user-agent="Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36" \
  --url=https://example.com
```

Windows:
```cmd
Release\freeprofile-browser.exe
```

Headless / off-screen (renders without an X11/Win32 window; useful for CI or VMs):
```bash
./build/Release/freeprofile-browser --url=https://example.com --windowless --disable-gpu
```

## Login UI

- Default credentials: `admin` / `admin`.
- After login, create profiles with a name, optional proxy, optional user-agent, and start URL.
- Click **Launch** to open that profile in a new browser instance with isolated cache.
- Profiles are saved to `<data-dir>/profiles.json`.

## Notes

- This is an **MVP scaffold**. A production anti-detect browser still needs:
  - per-profile fingerprint spoofing (Canvas, WebGL, WebRTC, fonts, timezone),
  - a cloud backend for profile sync,
  - an automation API.
- CEF binary distributions are large (~300 MB); they are not committed to this repo.
