// Copyright (c) 2024 FreeProfile Browser. All rights reserved.

#ifndef FREEPROFILE_BROWSER_APP_H_
#define FREEPROFILE_BROWSER_APP_H_

#include "include/cef_app.h"

class SimpleApp : public CefApp, public CefBrowserProcessHandler {
 public:
  SimpleApp();

  // CefApp methods:
  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }

  // CefBrowserProcessHandler methods:
  void OnContextInitialized() override;
  CefRefPtr<CefClient> GetDefaultClient() override;

 private:
  IMPLEMENT_REFCOUNTING(SimpleApp);
};

#endif  // FREEPROFILE_BROWSER_APP_H_
