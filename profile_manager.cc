#include "profile_manager.h"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "include/cef_parser.h"

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
    p.id = item->GetString("id").ToString();
    p.name = item->GetString("name").ToString();
    p.proxy = item->GetString("proxy").ToString();
    p.user_agent = item->GetString("userAgent").ToString();
    p.url = item->GetString("url").ToString();
    profiles_.push_back(p);
  }
  return true;
}

bool ProfileManager::Save() {
  CefRefPtr<CefDictionaryValue> root = CefDictionaryValue::Create();
  CefRefPtr<CefListValue> list = CefListValue::Create();
  for (const auto& p : profiles_) {
    CefRefPtr<CefDictionaryValue> item = CefDictionaryValue::Create();
    item->SetString("id", p.id);
    item->SetString("name", p.name);
    item->SetString("proxy", p.proxy);
    item->SetString("userAgent", p.user_agent);
    item->SetString("url", p.url);
    list->SetDictionary(list->GetSize(), item);
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
