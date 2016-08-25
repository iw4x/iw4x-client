bitmrc = {
	settings = nil,
}

function bitmrc.setup(settings)
	if not settings.source then error("Missing source.") end

	bitmrc.settings = settings
end

function bitmrc.import()
	if not bitmrc.settings then error("Run bitmrc.setup first") end

	bitmrc.includes()
end

function bitmrc.includes()
	if not bitmrc.settings then error("Run bitmrc.setup first") end

	includedirs { path.join(bitmrc.settings.source, "BitMRC/include") }
end

function bitmrc.project()
	if not bitmrc.settings then error("Run bitmrc.setup first") end

	project "bitmrc"
		language "C++"

		includedirs
		{
			path.join(bitmrc.settings.source, "BitMRC/include"),
		}
		files
		{
			path.join(bitmrc.settings.source, "BitMRC/**.cpp"),
		}
		removefiles
		{
			-- path.join(bitmrc.settings.source, "src/**/*test.cc"),
			path.join(bitmrc.settings.source, "BitMRC/main.cpp"),
			path.join(bitmrc.settings.source, "BitMRC/tests/**"),
			path.join(bitmrc.settings.source, "BitMRC/Storage/**"),
			path.join(bitmrc.settings.source, "BitMRC/Debug/**"),
		}

		-- dependencies
		libcryptopp.import()

		-- not our code, ignore POSIX usage warnings for now
		defines { "_SCL_SECURE_NO_WARNINGS" }
		warnings "Off"

		-- always build as static lib, as we include our custom classes and therefore can't perform shared linking
		kind "StaticLib"
end
