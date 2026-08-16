#ifndef FREEPROFILE_BROWSER_PROFILE_MANAGER_H_
#define FREEPROFILE_BROWSER_PROFILE_MANAGER_H_

#include <string>
#include <vector>

#include "include/cef_base.h"
#include "include/cef_values.h"

struct Profile {
  std::string id;
  std::string name;
  std::string proxy;
  std::string user_agent;
  std::string url;

  // Anti-detect / fingerprint fields.
  std::string os = "Windows";
  std::string timezone = "America/New_York";
  std::string language = "en-US";
  int screen_width = 1920;
  int screen_height = 1080;
  bool canvas_noise = false;
  bool webgl_noise = false;
  bool disable_webrtc = true;
  bool enable_geolocation = false;
  bool chrome_spoof = true;
  int device_memory_gb = 8;
  double latitude = 40.7128;
  double longitude = -74.0060;
  double accuracy = 10.0;
  std::string webgl_vendor = "Google Inc. (NVIDIA)";
  std::string webgl_renderer = "ANGLE (NVIDIA, NVIDIA GeForce GTX 1660 Ti Direct3D11 vs_5_0 ps_5_0, D3D11)";

  // Automation integration.
  std::string automation_tool = "none";
  int automation_port = 0;

  // Deeper anti-detect fields.
  int hardware_concurrency = 8;
  int max_touch_points = 0;
  double battery_level = 0.85;
  double device_pixel_ratio = 1.0;
  bool audio_noise = false;
  bool client_rect_noise = false;
  bool plugins_spoof = true;

  // Inline JS automation macro executed in child browser pages.
  std::string automation_script;
};

class ProfileManager {
 public:
  explicit ProfileManager(const std::string& data_dir);

  bool Load();
  bool Save();

  std::vector<Profile> GetProfiles() const;
  Profile GetProfile(const std::string& id) const;
  std::string AddProfile(const Profile& p);
  bool DeleteProfile(const std::string& id);

  std::string GetDataDir() const { return data_dir_; }

 private:
  std::string data_dir_;
  std::vector<Profile> profiles_;
};

#endif
