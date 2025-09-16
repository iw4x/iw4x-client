boost = {
	source = path.join(dependencies.basePath, "boost"),
}

function boost.setup()
	-- Setup boost headers during premake generation
	local boostPath = boost.source
	local versionFile = path.join(boostPath, "boost", "version.hpp")

	if not os.isfile(versionFile) then
		print("setting up Boost headers...")

		local submoduleResult = os.execute('git submodule update --init --recursive deps/boost')
		if submoduleResult ~= 0 then
			print("warning: git submodule update failed, continuing anyway...")
		end

		if os.host() == "windows" then
			local bootstrapScript = path.join(boostPath, "bootstrap.bat")
			if os.isfile(bootstrapScript) then
				print("running bootstrap.bat...")
				local result = os.execute('cd /d "' .. boostPath .. '" && call bootstrap.bat')
				if result == 0 then
					local b2Path = path.join(boostPath, "b2.exe")
					if os.isfile(b2Path) then
						print("Running b2.exe headers...")
						os.execute('cd /d "' .. boostPath .. '" && b2.exe headers')
					else
						print("warning: b2.exe not found after bootstrap")
					end
				else
					print("error: bootstrap.bat failed")
				end
			else
				print("error: bootstrap.bat not found at " .. bootstrapScript)
			end
		else
			local bootstrapScript = path.join(boostPath, "bootstrap.bat")
			if os.isfile(bootstrapScript) then
				print("running bootstrap.bat with wine...")
				local result = os.execute('cd "' .. boostPath .. '" && wine bootstrap.bat')
				if result == 0 then
					local b2Path = path.join(boostPath, "b2.exe")
					if os.isfile(b2Path) then
						print("running b2.exe headers with wine...")
						os.execute('cd "' .. boostPath .. '" && wine b2.exe headers')
					else
						print("warning: b2.exe not found after bootstrap")
					end
				else
					print("error: bootstrap.bat failed with wine")
				end
			else
				print("error: bootstrap.bat not found at " .. bootstrapScript)
			end
		end
	else
		print("boost headers already set up")
	end
end

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

		warnings "Off"
end

table.insert(dependencies, boost)
