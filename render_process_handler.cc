#include "render_process_handler.h"

#include <charconv>
#include <fstream>
#include <sstream>

#include "include/cef_command_line.h"
#include "include/cef_path_util.h"
#include "include/cef_v8.h"

namespace {

std::string ReadFile(const std::string& path) {
  std::ifstream f(path);
  if (!f.is_open()) return "";
  std::stringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

std::string EscapeJS(const std::string& s) {
  std::string out;
  out.reserve(s.size());
  for (char c : s) {
    if (c == '\\' || c == '"') out.push_back('\\');
    out.push_back(c);
  }
  return out;
}

void Replace(std::string& haystack,
             const std::string& needle,
             const std::string& value) {
  size_t pos = 0;
  while ((pos = haystack.find(needle, pos)) != std::string::npos) {
    haystack.replace(pos, needle.size(), value);
    pos += value.size();
  }
}

std::string GetSwitch(CefRefPtr<CefCommandLine> cmd, const std::string& name,
                      const std::string& fallback) {
  CefString v = cmd->GetSwitchValue(name);
  if (v.empty()) return fallback;
  return v.ToString();
}

double GetDoubleSwitch(CefRefPtr<CefCommandLine> cmd, const std::string& name,
                        double fallback) {
  CefString v = cmd->GetSwitchValue(name);
  if (v.empty()) return fallback;
  std::string s = v.ToString();
  double out = fallback;
  auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), out);
  if (ec != std::errc() || ptr != s.data() + s.size()) {
    return fallback;
  }
  return out;
}

int GetIntSwitch(CefRefPtr<CefCommandLine> cmd, const std::string& name,
                 int fallback) {
  CefString v = cmd->GetSwitchValue(name);
  if (v.empty()) return fallback;
  std::string s = v.ToString();
  int out = fallback;
  auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), out);
  if (ec != std::errc() || ptr != s.data() + s.size()) {
    return fallback;
  }
  return out;
}

bool GetBoolSwitch(CefRefPtr<CefCommandLine> cmd, const std::string& name,
                   bool fallback) {
  CefString v = cmd->GetSwitchValue(name);
  if (v.empty()) return fallback;
  return v.ToString() == "1" || v.ToString() == "true";
}

std::string PlatformForOS(const std::string& os) {
  if (os == "macOS") return "MacIntel";
  if (os == "iOS") return "iPhone";
  if (os == "Android") return "Linux armv8l";
  if (os == "Linux") return "Linux x86_64";
  return "Win32";
}

std::string DefaultUAForOS(const std::string& os,
                            const std::string& language) {
  const std::string chrome = "134.0.0.0";
  if (os == "macOS") {
    return "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/" + chrome + " Safari/537.36";
  }
  if (os == "Linux") {
    return "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/" + chrome + " Safari/537.36";
  }
  if (os == "Android") {
    return "Mozilla/5.0 (Linux; Android 14; SM-G998B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/" + chrome + " Mobile Safari/537.36";
  }
  if (os == "iOS") {
    return "Mozilla/5.0 (iPhone; CPU iPhone OS 17_0 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) CriOS/" + chrome + " Mobile/15E148 Safari/604.1";
  }
  return "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/" + chrome + " Safari/537.36";
}

int SeedFromString(const std::string& s) {
  int h = 0;
  for (char c : s) {
    h = (h * 31 + static_cast<unsigned char>(c)) & 0x7FFFFFFF;
  }
  return h;
}

}  // namespace

SimpleRenderProcessHandler::SimpleRenderProcessHandler() {
  CefMessageRouterConfig config;
  message_router_ = CefMessageRouterRendererSide::Create(config);
}

void SimpleRenderProcessHandler::OnWebKitInitialized() {
  CefString exe_dir;
  if (CefGetPath(PK_DIR_EXE, exe_dir)) {
    std::string path = exe_dir.ToString() + "/ui/fingerprint.js";
    fingerprint_script_ = ReadFile(path);
  }
}

void SimpleRenderProcessHandler::OnContextCreated(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context) {
  message_router_->OnContextCreated(browser, frame, context);

  InjectFingerprint(context);
}

void SimpleRenderProcessHandler::InjectFingerprint(
    CefRefPtr<CefV8Context> context) {
  if (fingerprint_script_.empty()) {
    return;
  }

  CefRefPtr<CefCommandLine> cmd = CefCommandLine::GetGlobalCommandLine();
  std::string os = GetSwitch(cmd, "fp-os", "Windows");
  std::string language = GetSwitch(cmd, "fp-language", "en-US");
  std::string timezone = GetSwitch(cmd, "fp-timezone", "America/New_York");
  std::string user_agent = GetSwitch(cmd, "user-agent", "");
  if (user_agent.empty()) {
    user_agent = DefaultUAForOS(os, language);
  }
  int screen_width = GetIntSwitch(cmd, "fp-screen-width", 1920);
  int screen_height = GetIntSwitch(cmd, "fp-screen-height", 1080);
  bool canvas_noise = GetBoolSwitch(cmd, "fp-canvas-noise", false);
  bool webgl_noise = GetBoolSwitch(cmd, "fp-webgl-noise", false);
  bool disable_webrtc = GetBoolSwitch(cmd, "fp-disable-webrtc", true);
  std::string webgl_vendor = GetSwitch(cmd, "fp-webgl-vendor", "Google Inc. (NVIDIA)");
  std::string webgl_renderer = GetSwitch(cmd, "fp-webgl-renderer", "ANGLE (NVIDIA, NVIDIA GeForce GTX 1660 Ti Direct3D11 vs_5_0 ps_5_0, D3D11)");
  std::string profile_id = GetSwitch(cmd, "profile-dir", "default");
  bool enable_geolocation = GetBoolSwitch(cmd, "fp-enable-geolocation", false);
  bool chrome_spoof = GetBoolSwitch(cmd, "fp-chrome-spoof", true);
  int device_memory = GetIntSwitch(cmd, "fp-device-memory", 8);
  double latitude = GetDoubleSwitch(cmd, "fp-latitude", 40.7128);
  double longitude = GetDoubleSwitch(cmd, "fp-longitude", -74.0060);
  double accuracy = GetDoubleSwitch(cmd, "fp-accuracy", 10.0);
  int hardware_concurrency = GetIntSwitch(cmd, "fp-hardware-concurrency", 8);
  int max_touch_points = GetIntSwitch(cmd, "fp-max-touch-points", 0);
  double battery_level = GetDoubleSwitch(cmd, "fp-battery-level", 0.85);
  double device_pixel_ratio = GetDoubleSwitch(cmd, "fp-device-pixel-ratio", 1.0);
  bool audio_noise = GetBoolSwitch(cmd, "fp-audio-noise", false);
  bool client_rect_noise = GetBoolSwitch(cmd, "fp-client-rect-noise", false);
  bool plugins_spoof = GetBoolSwitch(cmd, "fp-plugins-spoof", true);

  std::string script = fingerprint_script_;
  Replace(script, "{{USER_AGENT}}", EscapeJS(user_agent));
  Replace(script, "{{PLATFORM}}", EscapeJS(PlatformForOS(os)));
  Replace(script, "{{LANGUAGE}}", EscapeJS(language));
  Replace(script, "{{TIMEZONE}}", EscapeJS(timezone));
  Replace(script, "{{SCREEN_WIDTH}}", std::to_string(screen_width));
  Replace(script, "{{SCREEN_HEIGHT}}", std::to_string(screen_height));
  Replace(script, "{{CANVAS_NOISE}}", canvas_noise ? "true" : "false");
  Replace(script, "{{WEBGL_NOISE}}", webgl_noise ? "true" : "false");
  Replace(script, "{{WEBGL_VENDOR}}", EscapeJS(webgl_vendor));
  Replace(script, "{{WEBGL_RENDERER}}", EscapeJS(webgl_renderer));
  Replace(script, "{{DISABLE_WEBRTC}}", disable_webrtc ? "true" : "false");
  Replace(script, "{{ENABLE_GEOLOCATION}}", enable_geolocation ? "true" : "false");
  Replace(script, "{{CHROME_SPOOF}}", chrome_spoof ? "true" : "false");
  Replace(script, "{{DEVICE_MEMORY}}", std::to_string(device_memory));
  Replace(script, "{{LATITUDE}}", std::to_string(latitude));
  Replace(script, "{{LONGITUDE}}", std::to_string(longitude));
  Replace(script, "{{ACCURACY}}", std::to_string(accuracy));
  Replace(script, "{{HARDWARE_CONCURRENCY}}", std::to_string(hardware_concurrency));
  Replace(script, "{{MAX_TOUCH_POINTS}}", std::to_string(max_touch_points));
  Replace(script, "{{BATTERY_LEVEL}}", std::to_string(battery_level));
  Replace(script, "{{DEVICE_PIXEL_RATIO}}", std::to_string(device_pixel_ratio));
  Replace(script, "{{AUDIO_NOISE}}", audio_noise ? "true" : "false");
  Replace(script, "{{CLIENT_RECT_NOISE}}", client_rect_noise ? "true" : "false");
  Replace(script, "{{PLUGINS_SPOOF}}", plugins_spoof ? "true" : "false");
  Replace(script, "{{SEED}}", std::to_string(SeedFromString(profile_id)));

  CefRefPtr<CefV8Value> retval;
  CefRefPtr<CefV8Exception> exception;
  context->Eval(script, "", 0, retval, exception);

  std::string automation_file = GetSwitch(cmd, "fp-automation-file", "");
  if (!automation_file.empty()) {
    std::string automation_script = ReadFile(automation_file);
    if (!automation_script.empty()) {
      std::string wrapped =
          "(function(){"
          "function __fp_automation_run__(){\n" + automation_script + "\n}"
          "if(document.readyState==='complete'||document.readyState==='interactive'){"
          "setTimeout(__fp_automation_run__,0);"
          "}else{"
          "document.addEventListener('DOMContentLoaded',__fp_automation_run__);"
          "}"
          "})();";
      context->Eval(wrapped, "", 0, retval, exception);
    }
  }
}

void SimpleRenderProcessHandler::OnContextReleased(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context) {
  message_router_->OnContextReleased(browser, frame, context);
}

bool SimpleRenderProcessHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {
  return message_router_->OnProcessMessageReceived(browser, frame, source_process,
                                                   message);
}
