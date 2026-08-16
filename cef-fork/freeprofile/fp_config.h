// Copyright 2026 FreeProfile Browser contributors.
// SPDX-License-Identifier: MIT
//
// Header-only helper for reading per-profile fingerprint switches inside the
// Blink renderer process.  Chromium/CEF propagates switches from the browser
// process to renderer child processes, so the same --fp-* values used by the
// current FreeProfile CEF wrapper can be read directly by the patched engine.

#ifndef THIRD_PARTY_BLINK_RENDERER_FREEPROFILE_FP_CONFIG_H_
#define THIRD_PARTY_BLINK_RENDERER_FREEPROFILE_FP_CONFIG_H_

#include <string>

#include "base/command_line.h"
#include "base/strings/string_number_conversions.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace freeprofile {

// Returns the ASCII value of a command-line switch, or |fallback| if missing.
inline std::string FpSwitch(const char* name,
                              const std::string& fallback = std::string()) {
  base::CommandLine* cmd = base::CommandLine::ForCurrentProcess();
  if (!cmd || !cmd->HasSwitch(name)) return fallback;
  return cmd->GetSwitchValueASCII(name);
}

inline bool FpSwitchBool(const char* name, bool fallback = false) {
  base::CommandLine* cmd = base::CommandLine::ForCurrentProcess();
  if (!cmd || !cmd->HasSwitch(name)) return fallback;
  std::string v = cmd->GetSwitchValueASCII(name);
  return v.empty() || v == "1" || v == "true" || v == "True";
}

inline int FpSwitchInt(const char* name, int fallback = 0) {
  base::CommandLine* cmd = base::CommandLine::ForCurrentProcess();
  if (!cmd || !cmd->HasSwitch(name)) return fallback;
  int value = fallback;
  base::StringToInt(cmd->GetSwitchValueASCII(name), &value);
  return value;
}

inline double FpSwitchDouble(const char* name, double fallback = 0.0) {
  base::CommandLine* cmd = base::CommandLine::ForCurrentProcess();
  if (!cmd || !cmd->HasSwitch(name)) return fallback;
  double value = fallback;
  base::StringToDouble(cmd->GetSwitchValueASCII(name), &value);
  return value;
}

// WTF::String helpers used inside Blink.
inline String FpWtfString(const char* name, const String& fallback = String()) {
  std::string value = FpSwitch(name);
  return value.empty() ? fallback : String(value.c_str());
}

}  // namespace freeprofile

#endif  // THIRD_PARTY_BLINK_RENDERER_FREEPROFILE_FP_CONFIG_H_
