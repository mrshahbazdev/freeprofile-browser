// FreeProfile Browser — CEF-based multi-profile browser MVP
// Build: CEF_ROOT set to prebuilt CEF binary distribution

#include "app.h"

#if defined(OS_LINUX)
#include <X11/Xlib.h>
#endif

#include "include/base/cef_logging.h"
#include "include/cef_command_line.h"

#if defined(OS_LINUX)
namespace {

int XErrorHandlerImpl(Display* display, XErrorEvent* event) {
  LOG(WARNING) << "X error received: type " << event->type << ", serial "
               << event->serial << ", error_code " << event->error_code
               << ", request_code " << event->request_code << ", minor_code "
               << event->minor_code;
  return 0;
}

int XIOErrorHandlerImpl(Display* display) {
  return 0;
}

}  // namespace
#endif

NO_STACK_PROTECTOR
int main(int argc, char* argv[]) {
  // Allow injection of Chromium switches (proxy, user-agent, profile-dir).
  std::vector<std::string> arg_storage;
  std::vector<char*> arg_ptrs;

  // First argument is the executable path.
  arg_storage.push_back(argv[0]);

  // Pass proxy and user-agent as Chromium switches so CEF applies them.
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg.find("--proxy=") == 0) {
      arg_storage.push_back("--proxy-server=" + arg.substr(8));
    } else if (arg.find("--user-agent=") == 0) {
      arg_storage.push_back(arg);
    } else {
      arg_storage.push_back(arg);
    }
  }
  for (auto& s : arg_storage) {
    arg_ptrs.push_back(&s[0]);
  }

  CefMainArgs main_args(static_cast<int>(arg_ptrs.size()), arg_ptrs.data());

  int exit_code = CefExecuteProcess(main_args, nullptr, nullptr);
  if (exit_code >= 0) {
    return exit_code;
  }

#if defined(OS_LINUX)
  XSetErrorHandler(XErrorHandlerImpl);
  XSetIOErrorHandler(XIOErrorHandlerImpl);
#endif

  CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
  command_line->InitFromArgv(static_cast<int>(arg_ptrs.size()), arg_ptrs.data());

  CefSettings settings;

  settings.no_sandbox = true;

  if (command_line->HasSwitch("windowless")) {
    settings.windowless_rendering_enabled = true;
  }

  // Profile isolation: each process uses a dedicated cache directory.
  CefString profile_dir = command_line->GetSwitchValue("profile-dir");
  if (!profile_dir.empty()) {
    CefString(&settings.cache_path) = profile_dir;
    CefString(&settings.root_cache_path) = profile_dir;
  }

  CefRefPtr<SimpleApp> app(new SimpleApp);

  if (!CefInitialize(main_args, settings, app.get(), nullptr)) {
    return CefGetExitCode();
  }

  CefRunMessageLoop();
  CefShutdown();

  return 0;
}
