#include "profile_manager.h"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "include/cef_parser.h"

namespace {

void ReadProfileFromDict(CefRefPtr<CefDictionaryValue> item, Profile* p) {
  if (!item || !p) return;
  p->id = item->GetString("id").ToString();
  p->name = item->GetString("name").ToString();
  p->proxy = item->GetString("proxy").ToString();
  p->user_agent = item->GetString("userAgent").ToString();
  p->url = item->GetString("url").ToString();
  p->os = item->GetString("os").ToString();
  p->timezone = item->GetString("timezone").ToString();
  p->language = item->GetString("language").ToString();
  p->screen_width = item->GetInt("screenWidth");
  p->screen_height = item->GetInt("screenHeight");
  p->canvas_noise = item->GetBool("canvasNoise");
  p->webgl_noise = item->GetBool("webglNoise");
  p->disable_webrtc = item->GetBool("disableWebrtc");
  p->enable_geolocation = item->GetBool("enableGeolocation");
  p->chrome_spoof = item->HasKey("chromeSpoof") ? item->GetBool("chromeSpoof") : true;
  int device_memory = item->GetInt("deviceMemoryGb");
  p->device_memory_gb = device_memory > 0 ? device_memory : 8;
  double lat = item->GetDouble("latitude");
  double lon = item->GetDouble("longitude");
  double acc = item->GetDouble("accuracy");
  p->latitude = lat != 0.0 ? lat : 40.7128;
  p->longitude = lon != 0.0 ? lon : -74.0060;
  p->accuracy = acc != 0.0 ? acc : 10.0;
  p->webgl_vendor = item->GetString("webglVendor").ToString();
  p->webgl_renderer = item->GetString("webglRenderer").ToString();
  p->automation_tool = item->GetString("automationTool").ToString();
  int port = item->GetInt("automationPort");
  p->automation_port = port >= 0 ? port : 0;

  p->hardware_concurrency = item->GetInt("hardwareConcurrency") > 0 ? item->GetInt("hardwareConcurrency") : 8;
  p->max_touch_points = item->GetInt("maxTouchPoints") >= 0 ? item->GetInt("maxTouchPoints") : 0;
  double battery = item->GetDouble("batteryLevel");
  p->battery_level = (battery >= 0.0 && battery <= 1.0) ? battery : 0.85;
  double dpr = item->GetDouble("devicePixelRatio");
  p->device_pixel_ratio = dpr > 0.0 ? dpr : 1.0;
  p->audio_noise = item->GetBool("audioNoise");
  p->client_rect_noise = item->GetBool("clientRectNoise");
  p->plugins_spoof = item->HasKey("pluginsSpoof") ? item->GetBool("pluginsSpoof") : true;
  p->automation_script = item->GetString("automationScript").ToString();
}

CefRefPtr<CefDictionaryValue> WriteProfileToDict(const Profile& p) {
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
  item->SetString("automationScript", p.automation_script);
  return item;
}

}  // namespace

ProfileManager::ProfileManager(const std::string& data_dir)
    : data_dir_(data_dir) {}

bool ProfileManager::Load() {
  profiles_.clear();
  std::ifstream f(data_dir_ + "/profiles.json");
  if (!f.is_open()) {
    return true;
  }
  std::string json((std::istreambuf_iterator<char>(f)),
                   std::istreambuf_iterator<char>());
  CefRefPtr<CefValue> root = CefParseJSON(json, JSON_PARSER_RFC);
  if (!root || !root->IsValid() || root->GetType() != VTYPE_DICTIONARY) {
    return false;
  }
  CefRefPtr<CefDictionaryValue> dict = root->GetDictionary();
  CefRefPtr<CefListValue> list = dict->GetList("profiles");
  if (!list) {
    return true;
  }
  for (size_t i = 0; i < list->GetSize(); ++i) {
    CefRefPtr<CefDictionaryValue> item = list->GetDictionary(i);
    if (!item) continue;
    Profile p;
    ReadProfileFromDict(item, &p);
    profiles_.push_back(p);
  }
  return true;
}

bool ProfileManager::Save() {
  CefRefPtr<CefDictionaryValue> root = CefDictionaryValue::Create();
  CefRefPtr<CefListValue> list = CefListValue::Create();
  for (const auto& p : profiles_) {
    list->SetDictionary(list->GetSize(), WriteProfileToDict(p));
  }
  root->SetList("profiles", list);

  CefRefPtr<CefValue> root_value = CefValue::Create();
  root_value->SetDictionary(root);
  std::string json = CefWriteJSON(root_value, JSON_WRITER_DEFAULT).ToString();
  std::ofstream f(data_dir_ + "/profiles.json");
  if (!f.is_open()) {
    return false;
  }
  f << json;
  return true;
}

std::vector<Profile> ProfileManager::GetProfiles() const {
  return profiles_;
}

Profile ProfileManager::GetProfile(const std::string& id) const {
  for (const auto& p : profiles_) {
    if (p.id == id) return p;
  }
  return {};
}

std::string ProfileManager::AddProfile(const Profile& p) {
  Profile copy = p;
  auto now = std::chrono::system_clock::now().time_since_epoch().count();
  copy.id = std::to_string(now) + "_" + std::to_string(rand());
  profiles_.push_back(copy);
  Save();
  return copy.id;
}

bool ProfileManager::DeleteProfile(const std::string& id) {
  for (auto it = profiles_.begin(); it != profiles_.end(); ++it) {
    if (it->id == id) {
      profiles_.erase(it);
      Save();
      return true;
    }
  }
  return false;
}
