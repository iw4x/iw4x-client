zstd = {
	settings = nil
}

function zstd.setup(settings)
	if not settings.source then error("Missing source.") end

	zstd.settings = settings

	if not zstd.settings.defines then zstd.settings.defines = {} end
end

function zstd.import()
	if not zstd.settings then error("You need to call zstd.setup first") end

	links { "zstd" }
	zstd.includes()
end

function zstd.includes()
	if not zstd.settings then error("You need to call zstd.setup first") end

	includedirs
	{
		path.join(zstd.settings.source, "lib"),
		path.join(zstd.settings.source, "lib/common"),
		path.join(zstd.settings.source, "zlibWrapper")
	}
	defines(zstd.settings.defines)
end

function zstd.project()
	if not zstd.settings then error("You need to call zstd.setup first") end

	project "zstd"
		language "C"

		zstd.includes()
		files
		{
			path.join(zstd.settings.source, "lib/**.h"),
			path.join(zstd.settings.source, "lib/**.c"),
			path.join(zstd.settings.source, "zlibWrapper/zstd_zlibwrapper.h"),
			path.join(zstd.settings.source, "zlibWrapper/zstd_zlibwrapper.c"),
		}
		removefiles
		{
			path.join(zstd.settings.source, "lib/legacy/**.*"),
			--path.join(zstd.settings.source, "zlibWrapper/examples/**.*"),
		}
		defines
		{
			"zstd_DLL",
			"_CRT_SECURE_NO_DEPRECATE",
		}
		
		zlib.import()

		-- not our code, ignore POSIX usage warnings for now
		warnings "Off"
		
		--configuration "*Static"
			defines { "_LIB" }
			removedefines { "_USRDLL", "_DLL", "zstd_DLL" }
			kind "StaticLib"
end