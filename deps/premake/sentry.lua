sentry = {
	source = path.join(dependencies.basePath, "sentry-native"),
}

sentry.sources = {
	core = {
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
	},

	windows = {
		"src/sentry_random.c",
		"src/sentry_thread_stackwalk_windows.c",
		"src/sentry_windows_dbghelp.c",
		"src/modulefinder/sentry_modulefinder_windows.c",
		"src/path/sentry_path_windows.c",
		"src/process/sentry_process_windows.c",
		"src/symbolizer/sentry_symbolizer_windows.c",
	},

	backend = {
		"src/backends/sentry_backend_native.c",
		"src/backends/native/sentry_crash_handler.c",
		"src/backends/native/sentry_crash_ipc.c",
	},

	integration = {
		"src/integrations/sentry_integration_wer.c",
	},

	daemon = {
		"src/backends/native/sentry_crash_daemon.c",
		"src/backends/native/minidump/sentry_minidump_windows.c",
	},

	wer = {
		"src/backends/native/sentry_wer.c",
	},
}

sentry.defines = {
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
	"_CRT_NONSTDC_NO_DEPRECATE",
}

sentry.links = {"dbghelp", "shlwapi", "version", "winhttp"}

function sentry.import()
	links "sentry"
	links(sentry.links)

	defines {"SENTRY_BUILD_STATIC"}

	sentry.includes()
end

function sentry.includes()
	includedirs {
		path.join(sentry.source, "include"),
	}
end

function sentry.files(...)
	local resolved = {}

	for _, set in ipairs({...}) do
		for _, file in ipairs(set) do
			table.insert(resolved, path.join(sentry.source, file))
		end
	end

	files(resolved)
end

function sentry.common()
	language "C"
	cdialect "C11"

	includedirs {
		path.join(sentry.source, "include"),
		path.join(sentry.source, "src"),
		path.join(sentry.source, "src/backends/native"),
	}

	defines(sentry.defines)

	warnings "Off"
end

function sentry.project()
	project "sentry"
		kind "StaticLib"

		sentry.common()
		sentry.files(sentry.sources.core,
		             sentry.sources.windows,
		             sentry.sources.backend,
		             sentry.sources.integration)

		defines {"SENTRY_INTEGRATION_WER"}

		dependson {"sentry-crash", "sentry-wer"}

	project "sentry-crash"
		kind "ConsoleApp"

		sentry.common()
		sentry.files(sentry.sources.core,
		             sentry.sources.windows,
		             sentry.sources.backend,
		             sentry.sources.daemon)

		defines {"SENTRY_CRASH_DAEMON_STANDALONE"}

		links(sentry.links)

	project "sentry-wer"
		kind "SharedLib"

		sentry.common()
		sentry.files(sentry.sources.wer)

		linkoptions {"/DEF:\"" .. path.getabsolute(path.join(sentry.source, "src/backends/native/sentry_wer.def")) .. "\""}

		links {"wer"}
end

table.insert(dependencies, sentry)
