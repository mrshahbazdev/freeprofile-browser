#ifndef FREEPROFILE_BROWSER_MESSAGE_HANDLER_H_
#define FREEPROFILE_BROWSER_MESSAGE_HANDLER_H_

#include <memory>
#include <string>

#include "include/base/cef_ref_counted.h"
#include "include/wrapper/cef_message_router.h"

class ProfileManager;

class MessageHandler : public CefMessageRouterBrowserSide::Handler {
 public:
  explicit MessageHandler(ProfileManager* profile_manager);

  bool OnQuery(CefRefPtr<CefBrowser> browser,
               CefRefPtr<CefFrame> frame,
               int64_t query_id,
               const CefString& request,
               bool persistent,
               CefRefPtr<Callback> callback) override;

 private:
  std::string BuildLoginResponse(bool ok);
  std::string BuildProfilesResponse();
  std::string BuildAddProfileResponse(const CefString& id);
  std::string BuildOkResponse();

  ProfileManager* profile_manager_;
};

#endif
