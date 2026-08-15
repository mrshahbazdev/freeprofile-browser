#include "render_process_handler.h"

SimpleRenderProcessHandler::SimpleRenderProcessHandler() {
  CefMessageRouterConfig config;
  message_router_ = CefMessageRouterRendererSide::Create(config);
}

void SimpleRenderProcessHandler::OnContextCreated(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context) {
  message_router_->OnContextCreated(browser, frame, context);
}

void SimpleRenderProcessHandler::OnContextReleased(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context) {
  message_router_->OnContextReleased(browser, frame, context);
}

bool SimpleRenderProcessHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {
  return message_router_->OnProcessMessageReceived(browser, frame, source_process,
                                                   message);
}
