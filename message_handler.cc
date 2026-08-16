#include "message_handler.h"

#include <cstdlib>

#if defined(OS_WIN)
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "include/cef_parser.h"
#include "include/cef_values.h"
#include "handler.h"
#include "profile_manager.h"

MessageHandler::MessageHandler(ProfileManager* profile_manager)
    : profile_manager_(profile_manager) {}

namespace {

CefRefPtr<CefDictionaryValue> ParseRequest(const std::string& request) {
  CefRefPtr<CefValue> root = CefParseJSON(request, JSON_PARSER_RFC);
  if (!root || !root->IsValid() || root->GetType() != VTYPE_DICTIONARY) {
    return nullptr;
  }
  return root->GetDictionary();
}

std::string ProfilesToJSON(const std::vector<Profile>& profiles) {
  CefRefPtr<CefListValue> list = CefListValue::Create();
  for (const auto& p : profiles) {
    CefRefPtr<CefDictionaryValue> item = CefDictionaryValue::Create();
    item->SetString("id", p.id);
    item->SetString("name", p.name);
    item->SetString("proxy", p.proxy);
    item->SetString("userAgent", p.user_agent);
    item->SetString("url", p.url);
    item->SetString("os", p.os);
    item->SetString("timezone", p.timezone);
    item->SetString("language", p.language);
    item->SetInt("screenWidth", p.screen_width);
    item->SetInt("screenHeight", p.screen_height);
    item->SetBool("canvasNoise", p.canvas_noise);
    item->SetBool("webglNoise", p.webgl_noise);
    item->SetBool("disableWebrtc", p.disable_webrtc);
    item->SetBool("enableGeolocation", p.enable_geolocation);
    item->SetBool("chromeSpoof", p.chrome_spoof);
    item->SetInt("deviceMemoryGb", p.device_memory_gb);
    item->SetDouble("latitude", p.latitude);
    item->SetDouble("longitude", p.longitude);
    item->SetDouble("accuracy", p.accuracy);
    item->SetString("webglVendor", p.webgl_vendor);
    item->SetString("webglRenderer", p.webgl_renderer);
    item->SetString("automationTool", p.automation_tool);
    item->SetInt("automationPort", p.automation_port);
    item->SetInt("hardwareConcurrency", p.hardware_concurrency);
    item->SetInt("maxTouchPoints", p.max_touch_points);
    item->SetDouble("batteryLevel", p.battery_level);
    item->SetDouble("devicePixelRatio", p.device_pixel_ratio);
    item->SetBool("audioNoise", p.audio_noise);
    item->SetBool("clientRectNoise", p.client_rect_noise);
    item->SetBool("pluginsSpoof", p.plugins_spoof);
    list->SetDictionary(list->GetSize(), item);
  }
  CefRefPtr<CefValue> value = CefValue::Create();
  value->SetList(list);
  return CefWriteJSON(value, JSON_WRITER_DEFAULT).ToString();
}

std::string GetExePath() {
#if defined(OS_WIN)
  char path[MAX_PATH];
  GetModuleFileNameA(nullptr, path, MAX_PATH);
  return std::string(path);
#else
  char path[4096];
  ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
  if (len == -1) return "./freeprofile-browser";
  path[len] = '\0';
  return std::string(path);
#endif
}

void LaunchChild(const std::string& exe,
                 const Profile& p,
                 const std::string& data_dir) {
  std::string cmd = "\"" + exe + "\"";
  std::string profile_dir = data_dir + "/profiles/" + p.id;
  cmd += " --profile-dir=\"" + profile_dir + "\"";
  if (!p.proxy.empty()) {
    cmd += " --proxy=\"" + p.proxy + "\"";
  }
  if (!p.user_agent.empty()) {
    cmd += " --user-agent=\"" + p.user_agent + "\"";
  }
  cmd += " --url=\"" + p.url + "\"";

  cmd += " --fp-os=\"" + p.os + "\"";
  cmd += " --fp-timezone=\"" + p.timezone + "\"";
  cmd += " --timezone=\"" + p.timezone + "\"";
  cmd += " --fp-language=\"" + p.language + "\"";
  cmd += " --lang=\"" + p.language + "\"";
  cmd += " --fp-screen-width=\"" + std::to_string(p.screen_width) + "\"";
  cmd += " --fp-screen-height=\"" + std::to_string(p.screen_height) + "\"";
  cmd += " --fp-canvas-noise=\"" + std::string(p.canvas_noise ? "1" : "0") + "\"";
  cmd += " --fp-webgl-noise=\"" + std::string(p.webgl_noise ? "1" : "0") + "\"";
  cmd += " --fp-webgl-vendor=\"" + p.webgl_vendor + "\"";
  cmd += " --fp-webgl-renderer=\"" + p.webgl_renderer + "\"";
  cmd += " --fp-disable-webrtc=\"" + std::string(p.disable_webrtc ? "1" : "0") + "\"";
  cmd += " --fp-enable-geolocation=\"" + std::string(p.enable_geolocation ? "1" : "0") + "\"";
  cmd += " --fp-chrome-spoof=\"" + std::string(p.chrome_spoof ? "1" : "0") + "\"";
  cmd += " --fp-device-memory=\"" + std::to_string(p.device_memory_gb) + "\"";
  cmd += " --fp-latitude=\"" + std::to_string(p.latitude) + "\"";
  cmd += " --fp-longitude=\"" + std::to_string(p.longitude) + "\"";
  cmd += " --fp-accuracy=\"" + std::to_string(p.accuracy) + "\"";

  cmd += " --fp-hardware-concurrency=\"" + std::to_string(p.hardware_concurrency) + "\"";
  cmd += " --fp-max-touch-points=\"" + std::to_string(p.max_touch_points) + "\"";
  cmd += " --fp-battery-level=\"" + std::to_string(p.battery_level) + "\"";
  cmd += " --fp-device-pixel-ratio=\"" + std::to_string(p.device_pixel_ratio) + "\"";
  cmd += " --fp-audio-noise=\"" + std::string(p.audio_noise ? "1" : "0") + "\"";
  cmd += " --fp-client-rect-noise=\"" + std::string(p.client_rect_noise ? "1" : "0") + "\"";
  cmd += " --fp-plugins-spoof=\"" + std::string(p.plugins_spoof ? "1" : "0") + "\"";

  if (p.automation_port > 0) {
    cmd += " --remote-debugging-port=" + std::to_string(p.automation_port);
    cmd += " --remote-allow-origins=*";
  }

#if defined(OS_WIN)
  cmd += " & exit";
#else
  cmd += " &";
#endif
  if (std::system(cmd.c_str()) == -1) {
    // Ignore system() failure; launch best-effort.
  }
}

}  // namespace

bool MessageHandler::OnQuery(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             int64_t query_id,
                             const CefString& request,
                             bool persistent,
                             CefRefPtr<Callback> callback) {
  std::string req_str = request.ToString();
  CefRefPtr<CefDictionaryValue> req = ParseRequest(req_str);
  if (!req) {
    callback->Failure(1, "invalid request");
    return true;
  }

  std::string cmd = req->GetString("cmd").ToString();
  CefRefPtr<CefDictionaryValue> data = req->GetDictionary("data");
  if (!data) data = CefDictionaryValue::Create();

  if (cmd == "login") {
    std::string username = data->GetString("username").ToString();
    std::string password = data->GetString("password").ToString();
    if (username == "admin" && password == "admin") {
      callback->Success(BuildLoginResponse(true));
    } else {
      callback->Failure(2, "invalid credentials");
    }
    return true;
  }

  if (cmd == "getProfiles") {
    callback->Success(BuildProfilesResponse());
    return true;
  }

  if (cmd == "addProfile") {
    Profile p;
    p.name = data->GetString("name").ToString();
    p.proxy = data->GetString("proxy").ToString();
    p.user_agent = data->GetString("userAgent").ToString();
    p.url = data->GetString("url").ToString();
    p.os = data->GetString("os").ToString();
    p.timezone = data->GetString("timezone").ToString();
    p.language = data->GetString("language").ToString();
    p.screen_width = data->GetInt("screenWidth");
    p.screen_height = data->GetInt("screenHeight");
    p.canvas_noise = data->GetBool("canvasNoise");
    p.webgl_noise = data->GetBool("webglNoise");
    p.disable_webrtc = data->GetBool("disableWebrtc");
    p.enable_geolocation = data->GetBool("enableGeolocation");
    p.chrome_spoof = data->GetBool("chromeSpoof");
    p.device_memory_gb = data->GetInt("deviceMemoryGb");
    p.latitude = data->GetDouble("latitude");
    p.longitude = data->GetDouble("longitude");
    p.accuracy = data->GetDouble("accuracy");
    p.webgl_vendor = data->GetString("webglVendor").ToString();
    p.webgl_renderer = data->GetString("webglRenderer").ToString();
    p.automation_tool = data->GetString("automationTool").ToString();
    p.automation_port = data->GetInt("automationPort");
    p.hardware_concurrency = data->GetInt("hardwareConcurrency") > 0 ? data->GetInt("hardwareConcurrency") : 8;
    p.max_touch_points = data->GetInt("maxTouchPoints") >= 0 ? data->GetInt("maxTouchPoints") : 0;
    double battery = data->GetDouble("batteryLevel");
    p.battery_level = (battery >= 0.0 && battery <= 1.0) ? battery : 0.85;
    double dpr = data->GetDouble("devicePixelRatio");
    p.device_pixel_ratio = dpr > 0.0 ? dpr : 1.0;
    p.audio_noise = data->GetBool("audioNoise");
    p.client_rect_noise = data->GetBool("clientRectNoise");
    p.plugins_spoof = data->GetBool("pluginsSpoof");
    std::string id = profile_manager_->AddProfile(p);
    callback->Success(BuildAddProfileResponse(id));
    return true;
  }

  if (cmd == "deleteProfile") {
    std::string id = data->GetString("id").ToString();
    if (profile_manager_->DeleteProfile(id)) {
      callback->Success(BuildOkResponse());
    } else {
      callback->Failure(3, "profile not found");
    }
    return true;
  }

  if (cmd == "repaint") {
    if (auto* handler = SimpleHandler::GetInstance()) {
      handler->TriggerRepaint(browser);
    }
    callback->Success(BuildOkResponse());
    return true;
  }

  if (cmd == "launchProfile") {
    std::string id = data->GetString("id").ToString();
    Profile p = profile_manager_->GetProfile(id);
    if (p.id.empty()) {
      callback->Failure(4, "profile not found");
      return true;
    }
    std::string exe = GetExePath();
    std::string data_dir = profile_manager_->GetDataDir();
    LaunchChild(exe, p, data_dir);
    callback->Success(BuildOkResponse());
    return true;
  }

  return false;
}

std::string MessageHandler::BuildLoginResponse(bool ok) {
  CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
  dict->SetString("status", ok ? "ok" : "fail");
  CefRefPtr<CefValue> value = CefValue::Create();
  value->SetDictionary(dict);
  return CefWriteJSON(value, JSON_WRITER_DEFAULT).ToString();
}

std::string MessageHandler::BuildProfilesResponse() {
  return ProfilesToJSON(profile_manager_->GetProfiles());
}

std::string MessageHandler::BuildAddProfileResponse(const CefString& id) {
  CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
  dict->SetString("id", id);
  CefRefPtr<CefValue> value = CefValue::Create();
  value->SetDictionary(dict);
  return CefWriteJSON(value, JSON_WRITER_DEFAULT).ToString();
}

std::string MessageHandler::BuildOkResponse() {
  CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
  dict->SetString("status", "ok");
  CefRefPtr<CefValue> value = CefValue::Create();
  value->SetDictionary(dict);
  return CefWriteJSON(value, JSON_WRITER_DEFAULT).ToString();
}
