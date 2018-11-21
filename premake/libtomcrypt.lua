libtomcrypt = {
	settings = nil
}

function libtomcrypt.setup(settings)
	if not settings.source then error("Missing source") end

	libtomcrypt.settings = settings

	if not libtomcrypt.settings.defines then libtomcrypt.settings.defines = {} end
end

function libtomcrypt.import()
	if not libtomcrypt.settings then error("Run libtomcrypt.setup first") end

	links { "libtomcrypt" }
	libtomcrypt.includes()
end

function libtomcrypt.includes()
	if not libtomcrypt.settings then error("Run libtomcrypt.setup first") end

	defines(libtomcrypt.settings.defines)
	includedirs { path.join(libtomcrypt.settings.source, "src/headers") }
end

function libtomcrypt.project()
	if not libtomcrypt.settings then error("Run libtomcrypt.setup first") end

	project "libtomcrypt"
		language "C"

		libtomcrypt.includes()
		files
		{
			path.join(libtomcrypt.settings.source, "src/**.c"),
		}
		removefiles
		{
			path.join(libtomcrypt.settings.source, "src/**/*tab.c"),				-- included by files as necessary already afaik
			path.join(libtomcrypt.settings.source, "src/encauth/ocb3/**.c"),		-- fails in Visual Studio with invalid syntax
		}
		defines
		{
			"_CRT_SECURE_NO_WARNINGS",
			"LTC_SOURCE", -- we are compiling from source code
		}

		-- dependencies
		if libtommath and libtommath.settings then
			defines { "USE_LTM" }
			libtommath.import()
		end

		-- not our code, ignore POSIX usage warnings for now
		warnings "Off"

		defines { "_LIB" }
		removedefines { "_DLL", "_USRDLL" }
		linkoptions { "-IGNORE:4221" }
		kind "StaticLib"
end
