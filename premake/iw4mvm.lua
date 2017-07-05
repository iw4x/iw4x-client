iw4mvm = {
	settings = nil
}

function iw4mvm.setup(settings)
	if not settings.source then error("Missing source.") end

	iw4mvm.settings = settings

	if not iw4mvm.settings.defines then iw4mvm.settings.defines = {} end
end

function iw4mvm.import()
	if not iw4mvm.settings then error("You need to call iw4mvm.setup first") end

	links { "iw4mvm" }
	iw4mvm.includes()
end

function iw4mvm.includes()
	if not iw4mvm.settings then error("You need to call iw4mvm.setup first") end

	includedirs { iw4mvm.settings.source }
	libdirs { path.join(iw4mvm.settings.source, "IW4MVM") }
	defines(iw4mvm.settings.defines)
end

function iw4mvm.project()
	if not iw4mvm.settings then error("You need to call iw4mvm.setup first") end

	project "iw4mvm"
		language "C++"

		characterset ("MBCS")

		defines("_CRT_SECURE_NO_WARNINGS")

		iw4mvm.includes()
		files
		{
			path.join(iw4mvm.settings.source, "IW4MVM/*.h"),
			path.join(iw4mvm.settings.source, "IW4MVM/*.cpp"),
		}

		removefiles
		{
			--path.join(iw4mvm.settings.source, "IW4MVM/detours.cpp"),
			path.join(iw4mvm.settings.source, "IW4MVM/DllMain.cpp"),
		}

		-- not our code, ignore POSIX usage warnings for now
		warnings "Off"
		kind "StaticLib"
end