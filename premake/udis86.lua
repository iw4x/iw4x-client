udis86 = {
	settings = nil
}

function udis86.setup(settings)
	if not settings.source then error("Missing source.") end

	udis86.settings = settings

	if not udis86.settings.defines then udis86.settings.defines = {} end
end

function udis86.import()
	if not udis86.settings then error("You need to call udis86.setup first") end

	links { "udis86" }
	udis86.includes()
end

function udis86.includes()
	if not udis86.settings then error("You need to call udis86.setup first") end

	includedirs
	{
		udis86.settings.source,
		path.join(udis86.settings.source, "libudis86/"),
		path.join(udis86.settings.source, "../extra/udis86/"),
		path.join(udis86.settings.source, "../extra/udis86/libudis86/")
	}
	defines(udis86.settings.defines)
end

function udis86.project()
	if not udis86.settings then error("You need to call udis86.setup first") end

	project "udis86"
		language "C"

		udis86.includes()
		files
		{
			path.join(udis86.settings.source, "libudis86/*.h"),
			path.join(udis86.settings.source, "libudis86/*.c"),
			path.join(udis86.settings.source, "../extra/udis86/libudis86/*.c"),
		}

		-- not our code, ignore POSIX usage warnings for now
		warnings "Off"

		kind "StaticLib"
end