// Copyright (c) 2024 FreeProfile Browser. All rights reserved.

#ifndef FREEPROFILE_BROWSER_APP_H_
#define FREEPROFILE_BROWSER_APP_H_

#include <memory>
#include <string>

#include "include/cef_app.h"
#include "handler.h"
#include "profile_manager.h"
#include "render_process_handler.h"

class SimpleApp : public CefApp,
                  public CefBrowserProcessHandler {
 public:
  SimpleApp(const std::string& data_dir, bool is_alloy_style);

  // CefApp methods:
  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }
  CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override;

  // CefBrowserProcessHandler methods:
  void OnContextInitialized() override;
  CefRefPtr<CefClient> GetDefaultClient() override;

 private:
  void EnsureHandler();

  const std::string data_dir_;
  const bool is_alloy_style_;
  std::unique_ptr<ProfileManager> profile_manager_;
  CefRefPtr<SimpleHandler> default_handler_;
  CefRefPtr<SimpleRenderProcessHandler> render_process_handler_;

  IMPLEMENT_REFCOUNTING(SimpleApp);
};

#endif  // FREEPROFILE_BROWSER_APP_H_
