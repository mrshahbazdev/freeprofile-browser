// Copyright (c) 2024 FreeProfile Browser. All rights reserved.

#include "app.h"

#include <string>
#include <vector>

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/cef_path_util.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "profile_manager.h"
#include "render_handler.h"

namespace {

class SimpleWindowDelegate : public CefWindowDelegate {
 public:
  SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view,
                       cef_runtime_style_t runtime_style,
                       cef_show_state_t initial_show_state)
      : browser_view_(browser_view),
        runtime_style_(runtime_style),
        initial_show_state_(initial_show_state) {}

  SimpleWindowDelegate(const SimpleWindowDelegate&) = delete;
  SimpleWindowDelegate& operator=(const SimpleWindowDelegate&) = delete;

  void OnWindowCreated(CefRefPtr<CefWindow> window) override {
    window->AddChildView(browser_view_);
    if (initial_show_state_ != CEF_SHOW_STATE_HIDDEN) {
      window->Show();
    }
  }

  void OnWindowDestroyed(CefRefPtr<CefWindow> window) override {
    browser_view_ = nullptr;
  }

  bool CanClose(CefRefPtr<CefWindow> window) override {
    CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
    if (browser) {
      return browser->GetHost()->TryCloseBrowser();
    }
    return true;
  }

  CefSize GetPreferredSize(CefRefPtr<CefView> view) override {
    return CefSize(1280, 800);
  }

  cef_show_state_t GetInitialShowState(CefRefPtr<CefWindow> window) override {
    return initial_show_state_;
  }

  cef_runtime_style_t GetWindowRuntimeStyle() override { return runtime_style_; }

 private:
  CefRefPtr<CefBrowserView> browser_view_;
  const cef_runtime_style_t runtime_style_;
  const cef_show_state_t initial_show_state_;

  IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
};

class SimpleBrowserViewDelegate : public CefBrowserViewDelegate {
 public:
  explicit SimpleBrowserViewDelegate(cef_runtime_style_t runtime_style)
      : runtime_style_(runtime_style) {}

  SimpleBrowserViewDelegate(const SimpleBrowserViewDelegate&) = delete;
  SimpleBrowserViewDelegate& operator=(const SimpleBrowserViewDelegate&) = delete;

  bool OnPopupBrowserViewCreated(CefRefPtr<CefBrowserView> browser_view,
                               CefRefPtr<CefBrowserView> popup_browser_view,
                               bool is_devtools) override {
    CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(
        popup_browser_view, runtime_style_, CEF_SHOW_STATE_NORMAL));
    return true;
  }

  cef_runtime_style_t GetBrowserRuntimeStyle() override { return runtime_style_; }

 private:
  const cef_runtime_style_t runtime_style_;

  IMPLEMENT_REFCOUNTING(SimpleBrowserViewDelegate);
};

std::string GetUIUrl(const std::string& page) {
  CefString exe_dir;
  if (!CefGetPath(PK_DIR_EXE, exe_dir)) {
    return "about:blank";
  }
  std::string path = exe_dir.ToString() + "/ui/" + page;
  return "file://" + path;
}

}  // namespace

SimpleApp::SimpleApp(const std::string& data_dir, bool is_alloy_style)
    : data_dir_(data_dir), is_alloy_style_(is_alloy_style) {
  render_process_handler_ = new SimpleRenderProcessHandler;
}

CefRefPtr<CefRenderProcessHandler> SimpleApp::GetRenderProcessHandler() {
  return render_process_handler_;
}

namespace {

void PropagateFingerprintSwitches(CefRefPtr<CefCommandLine> from,
                                   CefRefPtr<CefCommandLine> to) {
  const std::vector<const char*> kSwitches = {
      "fp-os",
      "fp-timezone",
      "fp-language",
      "fp-screen-width",
      "fp-screen-height",
      "fp-canvas-noise",
      "fp-webgl-noise",
      "fp-webgl-vendor",
      "fp-webgl-renderer",
      "fp-disable-webrtc",
      "user-agent",
  };
  for (const char* sw : kSwitches) {
    if (from->HasSwitch(sw)) {
      std::string value = from->GetSwitchValue(sw).ToString();
      to->AppendSwitchWithValue(sw, value);
    }
  }
}

}  // namespace

void SimpleApp::OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line) {}

void SimpleApp::OnBeforeChildProcessLaunch(
    CefRefPtr<CefCommandLine> command_line) {
  CefRefPtr<CefCommandLine> global = CefCommandLine::GetGlobalCommandLine();
  if (!global) {
    return;
  }
  PropagateFingerprintSwitches(global, command_line);
}

void SimpleApp::EnsureHandler() {
  if (!profile_manager_) {
    profile_manager_.reset(new ProfileManager(data_dir_));
    profile_manager_->Load();
  }
  if (!default_handler_) {
    default_handler_ = new SimpleHandler(is_alloy_style_, profile_manager_.get());
  }
}

void SimpleApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  EnsureHandler();

  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();

  cef_runtime_style_t runtime_style = CEF_RUNTIME_STYLE_DEFAULT;
  if (is_alloy_style_) {
    runtime_style = CEF_RUNTIME_STYLE_ALLOY;
  }

  CefRefPtr<SimpleHandler> handler = default_handler_;

  CefBrowserSettings browser_settings;

  std::string url = command_line->GetSwitchValue("url").ToString();
  if (url.empty()) {
    url = GetUIUrl("index.html");
  }

  const bool use_views = !command_line->HasSwitch("use-native") &&
                         !command_line->HasSwitch("windowless");

  if (use_views) {
    CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
        handler, url, browser_settings, nullptr, nullptr,
        new SimpleBrowserViewDelegate(runtime_style));

    CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(
        browser_view, runtime_style, CEF_SHOW_STATE_NORMAL));
  } else if (command_line->HasSwitch("windowless")) {
    CefRefPtr<SimpleRenderHandler> render_handler(new SimpleRenderHandler);
    handler->SetRenderHandler(render_handler, render_handler);

    CefWindowInfo window_info;
    window_info.SetAsWindowless(static_cast<CefWindowHandle>(0));
    CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings,
                                  nullptr, nullptr);
  } else {
    CefWindowInfo window_info;
#if defined(OS_WIN)
    window_info.SetAsPopup(nullptr, "freeprofile-browser");
#endif
    window_info.runtime_style = runtime_style;
    CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings,
                                  nullptr, nullptr);
  }
}

CefRefPtr<CefClient> SimpleApp::GetDefaultClient() {
  EnsureHandler();
  return default_handler_;
}
