zlib = {
	source = path.join(dependencies.basePath, "zlib"),
}

function zlib.import()
	links {"zlib"}
	zlib.includes()
end

function zlib.includes()
	includedirs {
		zlib.source
	}

	defines {
		"ZLIB_CONST",
	}

	if os.host() ~= "windows" and os.isfile("/usr/include/unistd.h") then
		defines { "HAVE_UNISTD_H" }
	end
end

function zlib.project()
	project "zlib"
		language "C"

		zlib.includes()

		files {
			path.join(zlib.source, "*.h"),
			path.join(zlib.source, "*.c"),
		}

		defines {
			"_CRT_SECURE_NO_DEPRECATE",
		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, zlib)
