bitmrc = {
	settings = nil,
}

function bitmrc.setup(settings)
	if not settings.source then error("Missing source.") end

	bitmrc.settings = settings
end

function bitmrc.import()
	if not bitmrc.settings then error("Run bitmrc.setup first") end

	sqlite3.links()
	libcryptopp.links()
	bitmrc.links()
	bitmrc.includes()
end

function bitmrc.links()
	links { "bitmrc" }
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
			path.join(bitmrc.settings.source, "BitMRC/Storage/include"),
		}
		files
		{
			path.join(bitmrc.settings.source, "BitMRC/*.cpp"),
			path.join(bitmrc.settings.source, "BitMRC/Storage/*.cpp"),
		}
		removefiles
		{
			-- path.join(bitmrc.settings.source, "src/**/*test.cc"),
			path.join(bitmrc.settings.source, "BitMRC/main.*"),
			path.join(bitmrc.settings.source, "BitMRC/class.*"),
			path.join(bitmrc.settings.source, "BitMRC/tests/**"),
			
			path.join(bitmrc.settings.source, "BitMRC/Storage/Storable.cpp"),
		}

		-- dependencies
		sqlite3.import()
		libcryptopp.import()

		defines { "_SCL_SECURE_NO_WARNINGS" }
		warnings "Off"

		kind "StaticLib"
end
