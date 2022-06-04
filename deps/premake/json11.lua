json11 = {
	source = path.join(dependencies.basePath, "json11"),
}

function json11.import()
	links {"json11"}

	json11.includes()
end

function json11.includes()
	includedirs {json11.source}
end

function json11.project()
	project "json11"
		language "C++"
		cppdialect "C++11"

		files
		{
			path.join(json11.source, "*.cpp"),
			path.join(json11.source, "*.hpp"),
		}

		warnings "Off"

		defines {"_LIB"}
		removedefines {"_USRDLL", "_DLL"}
		kind "StaticLib"
end

table.insert(dependencies, json11)
