fmt = {
	settings = nil
}

function fmt.setup(settings)
	if not settings.source then error("Missing source.") end

	fmt.settings = settings
end

function fmt.import()
	if not fmt.settings then error("Run fmt.setup first") end

	links { "fmt" }
	fmt.includes()
end

function fmt.includes()
	if not fmt.settings then error("Run fmt.setup first") end

	includedirs { fmt.settings.source }
end

function fmt.project()
	if not fmt.settings then error("Run fmt.setup first") end

	project "fmt"
		language "C++"

		fmt.includes()

		files
		{
			path.join(fmt.settings.source, "fmt/*.cc"),
			path.join(fmt.settings.source, "fmt/*.h"),
		}

		-- not our code, ignore POSIX usage warnings for now
		warnings "Off"

		defines { "_LIB" }
		removedefines { "_USRDLL", "_DLL" }
		kind "StaticLib"
end
