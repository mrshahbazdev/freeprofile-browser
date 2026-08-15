// Copyright (c) 2024 FreeProfile Browser. All rights reserved.

#include "handler.h"

#include <sstream>
#include <string>
#include <vector>

#include "include/base/cef_callback.h"
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/cef_parser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "message_handler.h"
#include "profile_manager.h"

namespace {

SimpleHandler* g_instance = nullptr;

std::string GetDataURI(const std::string& data, const std::string& mime_type) {
  return "data:" + mime_type + ";base64," +
         CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
             .ToString();
}

// Parses proxy URL of the form scheme://[user:pass@]host:port
bool ParseProxy(const std::string& proxy_url,
                std::string* scheme,
                std::string* username,
                std::string* password,
                std::string* host,
                int* port) {
  if (proxy_url.empty()) return false;
  CefURLParts parts;
  if (!CefParseURL(proxy_url, parts)) return false;
  if (scheme) *scheme = CefString(&parts.scheme).ToString();
  if (username) *username = CefString(&parts.username).ToString();
  if (password) *password = CefString(&parts.password).ToString();
  if (host) *host = CefString(&parts.host).ToString();
  if (port) {
    CefString port_str(&parts.port);
    std::string ps = port_str.ToString();
    *port = ps.empty() ? 0 : std::stoi(ps);
  }
  return true;
}

}  // namespace

SimpleHandler::SimpleHandler(bool is_alloy_style,
                             ProfileManager* profile_manager)
    : is_alloy_style_(is_alloy_style), profile_manager_(profile_manager) {
  DCHECK(!g_instance);
  g_instance = this;

  CefMessageRouterConfig config;
  message_router_ = CefMessageRouterBrowserSide::Create(config);
  message_router_->AddHandler(new MessageHandler(profile_manager_), false);
}

SimpleHandler::~SimpleHandler() { g_instance = nullptr; }

SimpleHandler* SimpleHandler::GetInstance() { return g_instance; }

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  CEF_REQUIRE_UI_THREAD();
  if (auto browser_view = CefBrowserView::GetForBrowser(browser)) {
    CefRefPtr<CefWindow> window = browser_view->GetWindow();
    if (window) {
      window->SetTitle(title);
    }
  } else if (is_alloy_style_) {
    PlatformTitleChange(browser, title);
  }
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  browser_list_.push_back(browser);
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  if (browser_list_.size() == 1) {
    is_closing_ = true;
  }
  return false;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  message_router_->OnBeforeClose(browser);
  BrowserList::iterator bit = browser_list_.begin();
  for (; bit != browser_list_.end(); ++bit) {
    if ((*bit)->IsSame(browser)) {
      browser_list_.erase(bit);
      break;
    }
  }
  if (browser_list_.empty()) {
    CefQuitMessageLoop();
  }
}

void SimpleHandler::OnLoadStart(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                cef_transition_type_t transition_type) {}

void SimpleHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              int httpStatusCode) {
  CEF_REQUIRE_UI_THREAD();
  if (osr_handler_ && browser->GetHost()->IsWindowRenderingDisabled()) {
    osr_handler_->SetSaveNext(true);
    browser->GetHost()->WasResized();
    browser->GetHost()->Invalidate(PET_VIEW);
  }
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
  CEF_REQUIRE_UI_THREAD();
  if (!is_alloy_style_) {
    return;
  }
  if (errorCode == ERR_ABORTED) {
    return;
  }
  std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
        "<h2>Failed to load URL "
     << std::string(failedUrl) << " with error " << std::string(errorText)
     << " (" << errorCode << ").</h2></body></html>";
  frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}

CefRefPtr<CefRenderHandler> SimpleHandler::GetRenderHandler() {
  return render_handler_;
}

void SimpleHandler::SetRenderHandler(CefRefPtr<CefRenderHandler> handler,
                                      CefRefPtr<SimpleRenderHandler> osr_handler) {
  render_handler_ = handler;
  osr_handler_ = osr_handler;
}

bool SimpleHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {
  return message_router_->OnProcessMessageReceived(browser, frame, source_process,
                                                   message);
}

CefRefPtr<CefResourceRequestHandler> SimpleHandler::GetResourceRequestHandler(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    bool is_navigation,
    bool is_download,
    const CefString& request_initiator,
    bool& disable_default_handling) {
  return nullptr;
}

bool SimpleHandler::GetAuthCredentials(CefRefPtr<CefBrowser> browser,
                                       const CefString& origin_url,
                                       bool isProxy,
                                       const CefString& host,
                                       int port,
                                       const CefString& realm,
                                       const CefString& scheme,
                                       CefRefPtr<CefAuthCallback> callback) {
  if (!isProxy) return false;
  CefRefPtr<CefCommandLine> cmd = CefCommandLine::GetGlobalCommandLine();
  std::string proxy = cmd->GetSwitchValue("proxy").ToString();
  std::string user, pass, p_host;
  int p_port = 0;
  if (ParseProxy(proxy, nullptr, &user, &pass, &p_host, &p_port)) {
    if (p_host == host.ToString() && p_port == port && !user.empty()) {
      callback->Continue(user, pass);
      return true;
    }
  }
  return false;
}

bool SimpleHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   CefRefPtr<CefRequest> request,
                                   bool user_gesture,
                                   bool is_redirect) {
  message_router_->OnBeforeBrowse(browser, frame);
  return false;
}

void SimpleHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                              TerminationStatus status,
                                              int error_code,
                                              const CefString& error_string) {
  message_router_->OnRenderProcessTerminated(browser);
}

void SimpleHandler::ShowMainWindow() {
  if (!CefCurrentlyOn(TID_UI)) {
    CefPostTask(TID_UI, base::BindOnce(&SimpleHandler::ShowMainWindow, this));
    return;
  }
  if (browser_list_.empty()) {
    return;
  }
  auto main_browser = browser_list_.front();
  if (auto browser_view = CefBrowserView::GetForBrowser(main_browser)) {
    if (auto window = browser_view->GetWindow()) {
      window->Show();
    }
  } else if (is_alloy_style_) {
    PlatformShowWindow(main_browser);
  }
}

void SimpleHandler::TriggerRepaint(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  if (osr_handler_ && browser->GetHost()->IsWindowRenderingDisabled()) {
    osr_handler_->SetSaveNext(true);
    browser->GetHost()->WasResized();
    browser->GetHost()->Invalidate(PET_VIEW);
  }
}

void SimpleHandler::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    CefPostTask(TID_UI, base::BindOnce(&SimpleHandler::CloseAllBrowsers, this,
                                       force_close));
    return;
  }
  if (browser_list_.empty()) {
    return;
  }
  for (const auto& browser : browser_list_) {
    browser->GetHost()->CloseBrowser(force_close);
  }
}

void SimpleHandler::PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                                        const CefString& title) {}

void SimpleHandler::PlatformShowWindow(CefRefPtr<CefBrowser> browser) {}
