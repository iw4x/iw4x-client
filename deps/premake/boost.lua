boost = {
	source = path.join(dependencies.basePath, "boost"),
}

function boost.import()
	links {"boost"}
	boost.includes()
	boost.defines()
end

function boost.includes()
	includedirs {
		boost.source,
		-- symlinks might not work
		path.join(boost.source, "libs/*/include"),
	}
end

function boost.defines()
	defines {
		"BOOST_ALL_NO_LIB",
		"BOOST_ALL_STATIC_LINK",
	}
end

function boost.project()
	project "boost"
		language "C++"
		kind "Utility"
		location "%{wks.location}"

		boost.includes()

		files {
			path.join(boost.source, "bootstrap.sh"),
			path.join(boost.source, "bootstrap.bat"),
			path.join(boost.source, "Jamroot"),
		}

		filter "platforms:Win32"
			prebuildcommands {
				"cd /d \"$(ProjectDir)..\\deps\\boost\"",
				"if not exist boost\\version.hpp (",
				"    call bootstrap.bat",
				"    .\\b2.exe headers",
				")",
			}

		warnings "Off"
end

table.insert(dependencies, boost)
