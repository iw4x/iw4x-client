libcryptopp = {
	settings = nil,
}

function libcryptopp.setup(settings)
	if not settings.source then error("Missing source.") end

	libcryptopp.settings = settings
end

function libcryptopp.import()
	if not libcryptopp.settings then error("Run libcryptopp.setup first") end

	libcryptopp.includes()
end

function libcryptopp.includes()
	if not libcryptopp.settings then error("Run libcryptopp.setup first") end

  includedirs { libcryptopp.settings.source }
end

function libcryptopp.project()
	if not libcryptopp.settings then error("Run libcryptopp.setup first") end

	project "libcryptopp"
		language "C++"

		includedirs
		{
		  libcryptopp.settings.source
		}
		files
		{
			path.join(libcryptopp.settings.source, "src/**.cpp"),
		}
		removefiles
		{
			path.join(libcryptopp.settings.source, "TestData/**"),
			path.join(libcryptopp.settings.source, "TestVectors/**"),
		}

		-- not our code, ignore POSIX usage warnings for now
		defines { "_SCL_SECURE_NO_WARNINGS" }
		warnings "Off"

		-- always build as static lib, as we include our custom classes and therefore can't perform shared linking
		kind "StaticLib"
end
