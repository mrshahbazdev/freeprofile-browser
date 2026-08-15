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
  int next_id_ = 1;
};

#endif
