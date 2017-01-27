boost = {
	settings = nil,
}

function boost.setup(settings)
	if not settings.source then error("Missing source.") end

	boost.settings = settings
end

function boost.import()
	if not boost.settings then error("Run boost.setup first") end

	--links { "boost" }
	boost.includes()
end

function boost.includes()
	if not boost.settings then error("Run boost.setup first") end

	submodules = {
		"mpl",
		"core",
		"move",
		"tuple",
		"assert",
		"predef",
		"config",
		"detail",
		"winapi",
		"integer",
		"utility",
		"iterator",
		"container",
		"unordered",
		"date_time",
		"smart_ptr",
		"intrusive",
		"functional",
		"type_traits",
		"interprocess",
		"preprocessor",
		"static_assert",
		"throw_exception",
	}
	
	for i, submodule in ipairs(submodules) do
		includedirs { path.join(boost.settings.source, string.format("%s/include", submodule)) }
	end
	
	includedirs { boost.settings.source }
end

function boost.project()
	if not boost.settings then error("Run boost.setup first") end

--[[
	project "boost"
		language "C++"

		includedirs
		{
			boost.settings.source,
		}

		files
		{
			path.join(boost.settings.source, "*.cpp"),
			path.join(boost.settings.source, "*.hpp"),
		}
		removefiles
		{
			path.join(boost.settings.source, "test*"),
		}

		-- not our code, ignore POSIX usage warnings for now
		warnings "Off"

		defines { "_LIB" }
		removedefines { "_USRDLL", "_DLL" }
		kind "StaticLib"
]]
end
