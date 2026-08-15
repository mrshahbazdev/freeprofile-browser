#ifndef FREEPROFILE_BROWSER_RENDER_PROCESS_HANDLER_H_
#define FREEPROFILE_BROWSER_RENDER_PROCESS_HANDLER_H_

#include "include/cef_render_process_handler.h"
#include "include/wrapper/cef_message_router.h"

class SimpleRenderProcessHandler : public CefRenderProcessHandler {
 public:
  SimpleRenderProcessHandler();

  void OnWebKitInitialized() override;
  void OnContextCreated(CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        CefRefPtr<CefV8Context> context) override;
  void OnContextReleased(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         CefRefPtr<CefV8Context> context) override;
  bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefProcessId source_process,
                                CefRefPtr<CefProcessMessage> message) override;

 private:
  void InjectFingerprint(CefRefPtr<CefV8Context> context);

  CefRefPtr<CefMessageRouterRendererSide> message_router_;
  std::string fingerprint_script_;

  IMPLEMENT_REFCOUNTING(SimpleRenderProcessHandler);
};

#endif
