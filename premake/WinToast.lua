WinToast = {
	settings = nil,
}

function WinToast.setup(settings)
	if not settings.source then error("Missing source.") end

	WinToast.settings = settings
end

function WinToast.import()
	if not WinToast.settings then error("Run WinToast.setup first") end

	links { "WinToast" }
	WinToast.includes()
end

function WinToast.includes()
	if not WinToast.settings then error("Run WinToast.setup first") end

	includedirs { path.join(WinToast.settings.source, "src"), }
end

function WinToast.project()
	if not WinToast.settings then error("Run WinToast.setup first") end

	project "WinToast"
		language "C++"

		includedirs
		{
			WinToast.settings.source,
		}

		files
		{
			path.join(WinToast.settings.source, "src/*.cpp"),
			path.join(WinToast.settings.source, "src/*.h"),
		}

		-- not our code, ignore POSIX usage warnings for now
		warnings "Off"

		kind "StaticLib"
end
