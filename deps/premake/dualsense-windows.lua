dlwin = {
	source = path.join(dependencies.basePath, "dualsense-windows/VS19_Solution/DualSenseWindows"),
}

function dlwin.import()
	links {"dualsense-windows"}

	dlwin.includes()
end

function dlwin.includes()
	includedirs {
		path.join(dlwin.source, "include"),
	}
end


function dlwin.project()
	project "dualsense-windows"
		language "C++"

		defines {
			"DS5W_BUILD_LIB"
		}
		
		includedirs {
			path.join(dlwin.source, "include"),
			path.join(dlwin.source, "src"),
		}

		files
		{
			path.join(dlwin.source, "**.cpp"),
			path.join(dlwin.source, "**.h"),
		}

		warnings "Off"

		kind "StaticLib"
end

table.insert(dependencies, dlwin)
