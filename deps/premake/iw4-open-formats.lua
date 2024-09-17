iw4_open_formats = {
	source = path.join(dependencies.basePath, "iw4-open-formats"),
}

function iw4_open_formats.import()
	links "iw4-open-formats"

	iw4_open_formats.includes()
end

function iw4_open_formats.includes()
	includedirs {
		path.join(iw4_open_formats.source, "include")
	}
end

function iw4_open_formats.project()
	project "iw4-open-formats"
		language "C++"
	
		iw4_open_formats.includes()

		pchheader "std_include.hpp"
		pchsource (path.join(iw4_open_formats.source, "src/iw4-of/std_include.cpp"))

		files {
			path.join(iw4_open_formats.source, "src/iw4-of/**.hpp"),
			path.join(iw4_open_formats.source, "src/iw4-of/**.cpp"),
		}

		includedirs {
			path.join(iw4_open_formats.source, "src/iw4-of"),
			path.join(iw4_open_formats.source, "include"),
		}

		libtomcrypt.includes()
		libtommath.includes()
		rapidjson.includes()
		zlib.includes()

		kind "StaticLib"
end

table.insert(dependencies, iw4_open_formats)
