-- Sentry Native SDK with its native backend.
--
-- The client links the sentry library. Its native backend hands crashes
-- over to the crash daemon (sentry-crash) and registers the Windows Error
-- Reporting module (sentry-wer) for the crashes that never reach an
-- in-process handler, such as fail-fast exceptions. Both binaries go next
-- to the client, which is why the library depends on them for the build
-- order.
--
-- We list the sources since upstream picks them per platform and backend
-- in its CMake build, which we don't use.
--
local targets = ...

-- Sources that the library and the crash daemon share.
--
local common_sources =
{
  "src/sentry_alloc.c",
  "src/sentry_app_hang_latch.c",
  "src/sentry_app_hang_monitor.c",
  "src/sentry_attachment.c",
  "src/sentry_backend.c",
  "src/sentry_batcher.c",
  "src/sentry_client_report.c",
  "src/sentry_core.c",
  "src/sentry_database.c",
  "src/sentry_envelope.c",
  "src/sentry_hint.c",
  "src/sentry_info.c",
  "src/sentry_json.c",
  "src/sentry_logger.c",
  "src/sentry_logs.c",
  "src/sentry_metrics.c",
  "src/sentry_mpack.c",
  "src/sentry_options.c",
  "src/sentry_os.c",
  "src/sentry_ratelimiter.c",
  "src/sentry_retry.c",
  "src/sentry_ringbuffer.c",
  "src/sentry_scope.c",
  "src/sentry_session.c",
  "src/sentry_slice.c",
  "src/sentry_string.c",
  "src/sentry_sync.c",
  "src/sentry_telemetry.c",
  "src/sentry_tracing.c",
  "src/sentry_transport.c",
  "src/sentry_utils.c",
  "src/sentry_uuid.c",
  "src/sentry_value.c",
  "src/sentry_writer.c",
  "src/path/sentry_path.c",
  "src/screenshot/sentry_screenshot.c",
  "src/screenshot/sentry_screenshot_none.c",
  "src/session_replay/sentry_session_replay.c",
  "src/session_replay/sentry_session_replay_none.c",
  "src/transports/sentry_function_transport.c",
  "src/transports/sentry_http_transport.c",
  "src/transports/sentry_http_transport_winhttp.c",
  "src/unwinder/sentry_unwinder.c",
  "src/unwinder/sentry_unwinder_dbghelp.c",

  -- Windows.
  --
  "src/sentry_random.c",
  "src/sentry_thread_stackwalk_windows.c",
  "src/sentry_windows_dbghelp.c",
  "src/modulefinder/sentry_modulefinder_windows.c",
  "src/path/sentry_path_windows.c",
  "src/process/sentry_process_windows.c",
  "src/symbolizer/sentry_symbolizer_windows.c",

  -- Native backend.
  --
  "src/backends/sentry_backend_native.c",
  "src/backends/native/sentry_crash_handler.c",
  "src/backends/native/sentry_crash_ipc.c"
}

-- Include directories and preprocessor definitions of every Sentry target.
--
local common_includedirs =
{
  "include",
  "src",
  "src/backends/native"
}

local common_defines =
{
  "SENTRY_BUILD_STATIC",
  "SENTRY_WITH_NATIVE_BACKEND",
  "SENTRY_BACKEND_NATIVE",
  "SENTRY_WITH_UNWINDER_DBGHELP",
  "SENTRY_SCREENSHOT_NONE",
  "SENTRY_HANDLER_STACK_SIZE=64",
  "SENTRY_BATCHER_BUFFER_COUNT=3",
  "SENTRY_THREAD_STACK_GUARANTEE_FACTOR=10",
  "SENTRY_THREAD_STACK_GUARANTEE_AUTO_INIT",
  "SIZEOF_LONG=4",
  "_WIN32_WINNT=0x0601",
  "_CRT_SECURE_NO_WARNINGS",
  "_CRT_NONSTDC_NO_DEPRECATE"
}

local mingw_options = _OPTIONS["mingw"] and
  {'-include "' .. path.join (_MAIN_SCRIPT_DIR,
                              "deps/extra/sentry/mingw-thread.h") .. '"'} or nil

-- System libraries that the library and the crash daemon link.
--
local common_links =
{
  "dbghelp",
  "shlwapi",
  "version",
  "winhttp"
}

local function concat (...)
  local r = {}

  for _, l in ipairs ({...}) do
    for _, v in ipairs (l) do
      table.insert (r, v)
    end
  end

  return r
end

targets:static_library ("sentry",
{
  directory    = "sentry-native",
  language     = "C",
  cdialect     = "C11",
  buildoptions = mingw_options,
  dependson    = {"sentry-crash", "sentry-wer"},
  files        = concat (common_sources,
                        {"src/integrations/sentry_integration_wer.c"}),
  includedirs  = common_includedirs,
  defines      = concat (common_defines, {"SENTRY_INTEGRATION_WER"}),

  export =
  {
    includedirs = {"include"},
    defines     = {"SENTRY_BUILD_STATIC"},
    links       = common_links
  }
})

targets:executable ("sentry-crash",
{
  directory    = "sentry-native",
  language     = "C",
  cdialect     = "C11",
  buildoptions = mingw_options,
  includedirs  = common_includedirs,
  defines      = concat (common_defines, {"SENTRY_CRASH_DAEMON_STANDALONE"}),
  links        = common_links,

  files = concat (common_sources,
                  {"src/backends/native/sentry_crash_daemon.c",
                   "src/backends/native/minidump/sentry_minidump_windows.c"})
})

-- Note that the module definition file names the entry points that
-- Windows Error Reporting looks up. Note also that MinGW strips the
-- stdcall suffixes from the exports with --kill-at.
--
targets:shared_library ("sentry-wer",
{
  directory   = "sentry-native",
  language    = "C",
  cdialect    = "C11",
  includedirs = common_includedirs,
  defines     = common_defines,
  links       = {"wer"},
  linkoptions = _OPTIONS["mingw"] and {"-Wl,--kill-at"} or nil,

  files =
  {
    "src/backends/native/sentry_wer.c",
    "src/backends/native/sentry_wer.def"
  }
})
