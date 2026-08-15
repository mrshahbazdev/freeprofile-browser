#ifndef FREEPROFILE_BROWSER_RENDER_HANDLER_H_
#define FREEPROFILE_BROWSER_RENDER_HANDLER_H_

#include "include/cef_render_handler.h"

class SimpleRenderHandler : public CefRenderHandler {
 public:
  SimpleRenderHandler();

  void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
  bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) override;
  void OnPaint(CefRefPtr<CefBrowser> browser,
               PaintElementType type,
               const RectList& dirtyRects,
               const void* buffer,
               int width,
               int height) override;

  void SetSaveNext(bool save);

 private:
  bool save_next_ = false;
  IMPLEMENT_REFCOUNTING(SimpleRenderHandler);
};

#endif
