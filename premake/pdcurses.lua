pdcurses = {
	settings = nil,
}

function pdcurses.setup(settings)
	if not settings.source then error("Missing source.") end

	pdcurses.settings = settings
end

function pdcurses.import()
	if not pdcurses.settings then error("Run pdcurses.setup first") end

	links { "pdcurses" }
	pdcurses.includes()
end

function pdcurses.includes()
	if not pdcurses.settings then error("Run pdcurses.setup first") end

	includedirs { pdcurses.settings.source }
end

function pdcurses.project()
	if not pdcurses.settings then error("Run pdcurses.setup first") end

	project "pdcurses"
		language "C"

		includedirs
		{
			pdcurses.settings.source,
		}

		files
		{
			path.join(pdcurses.settings.source, "pdcurses/*.c"),
			path.join(pdcurses.settings.source, "pdcurses/*.h"),
			path.join(pdcurses.settings.source, "win32/*.c"),
			path.join(pdcurses.settings.source, "win32/*.h"),
		}

		-- not our code, ignore POSIX usage warnings for now
		warnings "Off"

		-- always build as static lib, as pdcurses doesn't export anything
		kind "StaticLib"
end
