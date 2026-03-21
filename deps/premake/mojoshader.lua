mojoshader = {
	source = path.join(dependencies.basePath, "mojoshader"),
	profilesSource = path.join(dependencies.basePath, "mojoshader/profiles"),
}

function mojoshader.import()
	links {"mojoshader"}
	dxsdk.import()
	mojoshader.includes()
end

function mojoshader.includes()
	includedirs {
		mojoshader.source
	}

	defines {
		"MOJOSHADER_NO_VERSION_INCLUDE"
	}

	dxsdk.includes()
end

function mojoshader.project()
	project "mojoshader"
		language "C"

		mojoshader.includes()

		files {
			path.join(mojoshader.source, "*.h"),
			path.join(mojoshader.source, "*.c"),
			path.join(mojoshader.profilesSource, "*.h"),
			path.join(mojoshader.profilesSource, "*.c"),
		}

		defines {
			"_CRT_SECURE_NO_DEPRECATE",
			"MOJOSHADER_NO_VERSION_INCLUDE"
		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, mojoshader)