#include "render_handler.h"

#include <iostream>

SimpleRenderHandler::SimpleRenderHandler() = default;

void SimpleRenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
  rect.Set(0, 0, 1280, 800);
}

bool SimpleRenderHandler::GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) {
  screen_info.Set(1.0f, 32, 8, false, CefRect(0, 0, 1280, 800),
                  CefRect(0, 0, 1280, 800));
  return true;
}

void SimpleRenderHandler::OnPaint(CefRefPtr<CefBrowser> browser,
                                  PaintElementType type,
                                  const RectList& dirtyRects,
                                  const void* buffer,
                                  int width,
                                  int height) {
  std::cerr << "OSR paint: " << width << "x" << height << std::endl;
  if (!save_next_ || type != PET_VIEW) {
    return;
  }
  save_next_ = false;

  const unsigned char* src = static_cast<const unsigned char*>(buffer);
  const int pixels = width * height;
  unsigned char* rgb = new unsigned char[pixels * 3];
  for (int i = 0; i < pixels; ++i) {
    rgb[i * 3 + 0] = src[i * 4 + 2];  // R
    rgb[i * 3 + 1] = src[i * 4 + 1];  // G
    rgb[i * 3 + 2] = src[i * 4 + 0];  // B
  }

  FILE* f = fopen("/tmp/osr_screenshot.ppm", "wb");
  if (f) {
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    fwrite(rgb, 1, pixels * 3, f);
    fclose(f);
    std::cerr << "Saved /tmp/osr_screenshot.ppm" << std::endl;
  }
  delete[] rgb;
}

void SimpleRenderHandler::SetSaveNext(bool save) {
  save_next_ = save;
}
