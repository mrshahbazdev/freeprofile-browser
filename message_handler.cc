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
