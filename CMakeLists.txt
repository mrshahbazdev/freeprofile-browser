cmake_minimum_required(VERSION 3.21)
project(freeprofile-browser C CXX)

if(NOT DEFINED CEF_ROOT)
  message(FATAL_ERROR "Set CEF_ROOT to your CEF binary distribution path.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake")
set(USE_SANDBOX OFF)
find_package(CEF REQUIRED)

SET_CEF_TARGET_OUT_DIR()

add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)

set(FREEPROFILE_SRCS
  app.cc
  app.h
  handler.cc
  handler.h
  message_handler.cc
  message_handler.h
  profile_manager.cc
  profile_manager.h
  render_handler.cc
  render_handler.h
  render_process_handler.cc
  render_process_handler.h
)

if(OS_WINDOWS)
  list(APPEND FREEPROFILE_SRCS main_win.cpp)
  add_executable(freeprofile-browser WIN32 ${FREEPROFILE_SRCS})
elseif(OS_LINUX)
  list(APPEND FREEPROFILE_SRCS main.cpp)
  add_executable(freeprofile-browser ${FREEPROFILE_SRCS})
elseif(OS_MAC)
  list(APPEND FREEPROFILE_SRCS main.cpp)
  add_executable(freeprofile-browser ${FREEPROFILE_SRCS})
endif()

ADD_LOGICAL_TARGET("cef_libcef" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")
SET_EXECUTABLE_TARGET_PROPERTIES(freeprofile-browser)
target_link_libraries(freeprofile-browser libcef_dll_wrapper cef_libcef ${CEF_STANDARD_LIBS})

if(OS_LINUX OR OS_WINDOWS)
  COPY_FILES(freeprofile-browser "${CEF_BINARY_FILES}" "${CEF_ROOT}/Release" "${CEF_TARGET_OUT_DIR}")
  COPY_FILES(freeprofile-browser "${CEF_RESOURCE_FILES}" "${CEF_ROOT}/Resources" "${CEF_TARGET_OUT_DIR}")
  add_custom_command(TARGET freeprofile-browser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/ui"
    "${CEF_TARGET_OUT_DIR}/ui")
endif()

if(OS_WINDOWS)
  # Add manifest and set as GUI application.
  set_target_properties(freeprofile-browser PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

PRINT_CEF_CONFIG()
