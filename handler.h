// Copyright (c) 2024 FreeProfile Browser. All rights reserved.

#ifndef FREEPROFILE_BROWSER_HANDLER_H_
#define FREEPROFILE_BROWSER_HANDLER_H_

#include <list>

#include "include/cef_client.h"
#include "include/wrapper/cef_message_router.h"
#include "render_handler.h"

class ProfileManager;

class SimpleHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
                      public CefRequestHandler {
 public:
  SimpleHandler(bool is_alloy_style, ProfileManager* profile_manager);
  ~SimpleHandler() override;

  static SimpleHandler* GetInstance();

  // CefClient methods:
  CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
  CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
  CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }
  CefRefPtr<CefRenderHandler> GetRenderHandler() override;
  bool OnProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) override;

  // CefDisplayHandler methods:
  void OnTitleChange(CefRefPtr<CefBrowser> browser,
                     const CefString& title) override;

  // CefLifeSpanHandler methods:
  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  bool DoClose(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

  // CefLoadHandler methods:
  void OnLoadStart(CefRefPtr<CefBrowser> browser,
                   CefRefPtr<CefFrame> frame,
                   cef_transition_type_t transition_type) override;
  void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                 CefRefPtr<CefFrame> frame,
                 int httpStatusCode) override;
  void OnLoadError(CefRefPtr<CefBrowser> browser,
                   CefRefPtr<CefFrame> frame,
                   ErrorCode errorCode,
                   const CefString& errorText,
                   const CefString& failedUrl) override;

  // CefRenderHandler methods:
  void SetRenderHandler(CefRefPtr<CefRenderHandler> handler,
                        CefRefPtr<SimpleRenderHandler> osr_handler);

  // CefRequestHandler methods:
  CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefRequest> request,
      bool is_navigation,
      bool is_download,
      const CefString& request_initiator,
      bool& disable_default_handling) override;
  bool GetAuthCredentials(CefRefPtr<CefBrowser> browser,
                           const CefString& origin_url,
                           bool isProxy,
                           const CefString& host,
                           int port,
                           const CefString& realm,
                           const CefString& scheme,
                           CefRefPtr<CefAuthCallback> callback) override;
  bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                      CefRefPtr<CefFrame> frame,
                      CefRefPtr<CefRequest> request,
                      bool user_gesture,
                      bool is_redirect) override;
  void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                 TerminationStatus status,
                                 int error_code,
                                 const CefString& error_string) override;

  void ShowMainWindow();
  void CloseAllBrowsers(bool force_close);
  void TriggerRepaint(CefRefPtr<CefBrowser> browser);
  bool IsClosing() const { return is_closing_; }

 private:
  void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title);
  void PlatformShowWindow(CefRefPtr<CefBrowser> browser);

  const bool is_alloy_style_;
  ProfileManager* profile_manager_;
  CefRefPtr<CefMessageRouterBrowserSide> message_router_;
  CefRefPtr<CefRenderHandler> render_handler_;
  CefRefPtr<SimpleRenderHandler> osr_handler_;
  typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
  BrowserList browser_list_;
  bool is_closing_ = false;

  IMPLEMENT_REFCOUNTING(SimpleHandler);
};

#endif  // FREEPROFILE_BROWSER_HANDLER_H_
