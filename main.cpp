// FreeProfile Browser — CEF-based multi-profile browser MVP
// Build: CEF_ROOT set to prebuilt CEF binary distribution

#include "app.h"

#include <filesystem>
#include <string>
#include <vector>

#if defined(OS_LINUX)
#include <unistd.h>
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

namespace {

std::string GetDefaultDataDir() {
#if defined(OS_LINUX)
  char path[4096];
  ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
  if (len != -1) {
    path[len] = '\0';
    std::string exe_path(path);
    size_t last_slash = exe_path.find_last_of('/');
    if (last_slash != std::string::npos) {
      return exe_path.substr(0, last_slash) + "/data";
    }
  }
#endif
  return "./data";
}

std::string GetSwitchValue(const std::vector<std::string>& args,
                            const std::string& name) {
  for (const auto& arg : args) {
    if (arg.find("--" + name + "=") == 0) {
      return arg.substr(name.size() + 3);
    }
    if (arg == "--" + name && &arg != &args.back()) {
      // Not used currently.
    }
  }
  return "";
}

bool HasSwitch(const std::vector<std::string>& args, const std::string& name) {
  for (const auto& arg : args) {
    if (arg == "--" + name || arg.find("--" + name + "=") == 0) {
      return true;
    }
  }
  return false;
}

}  // namespace

NO_STACK_PROTECTOR
int main(int argc, char* argv[]) {
  std::vector<std::string> arg_storage;
  std::vector<char*> arg_ptrs;

  arg_storage.push_back(argv[0]);

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg.find("--proxy=") == 0) {
      arg_storage.push_back("--proxy-server=" + arg.substr(8));
    } else if (arg.find("--user-agent=") == 0) {
      arg_storage.push_back(arg);
    } else if (arg.find("--fp-timezone=") == 0) {
      arg_storage.push_back(arg);
      arg_storage.push_back("--timezone=" + arg.substr(14));
    } else if (arg.find("--fp-language=") == 0) {
      arg_storage.push_back(arg);
      arg_storage.push_back("--lang=" + arg.substr(14));
    } else {
      arg_storage.push_back(arg);
    }
  }

  std::string data_dir = GetSwitchValue(arg_storage, "data-dir");
  if (data_dir.empty()) {
    data_dir = GetDefaultDataDir();
  }
  std::filesystem::create_directories(data_dir);

  bool is_alloy_style = HasSwitch(arg_storage, "use-alloy-style");

  CefRefPtr<SimpleApp> app(new SimpleApp(data_dir, is_alloy_style));

  for (auto& s : arg_storage) {
    arg_ptrs.push_back(&s[0]);
  }

  CefMainArgs main_args(static_cast<int>(arg_ptrs.size()), arg_ptrs.data());

  int exit_code = CefExecuteProcess(main_args, app.get(), nullptr);
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

  if (!CefInitialize(main_args, settings, app.get(), nullptr)) {
    return CefGetExitCode();
  }

  CefRunMessageLoop();
  CefShutdown();

  return 0;
}
