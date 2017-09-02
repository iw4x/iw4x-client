dht = {
	settings = nil
}

function dht.setup(settings)
	if not settings.source then error("Missing source.") end

	dht.settings = settings

	if not dht.settings.defines then dht.settings.defines = {} end
end

function dht.import()
	if not dht.settings then error("You need to call dht.setup first") end

	links { "dht" }
	dht.includes()
end

function dht.includes()
	if not dht.settings then error("You need to call dht.setup first") end

	includedirs { dht.settings.source }
	defines(dht.settings.defines)
end

function dht.project()
	if not dht.settings then error("You need to call dht.setup first") end

	project "dht"
		language "C"

		dht.includes()
		files
		{
			path.join(dht.settings.source, "dht.h"),
			path.join(dht.settings.source, "dht.c"),
		}

		-- not our code, ignore POSIX usage warnings for now
		warnings "Off"

		kind "StaticLib"
end