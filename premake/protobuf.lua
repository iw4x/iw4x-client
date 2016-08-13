protobuf = {
	settings = nil,
}

function protobuf.setup(settings)
	if not settings.source then error("Missing source.") end

	protobuf.settings = settings
end

function protobuf.import()
	if not protobuf.settings then error("Run protobuf.setup first") end

	links { "protobuf" }
	protobuf.includes()
end

function protobuf.includes()
	if not protobuf.settings then error("Run protobuf.setup first") end

	includedirs
	{
		path.join(protobuf.settings.source, "src"),
	}
end

function protobuf.project()
	if not protobuf.settings then error("Run protobuf.setup first") end

	project "protobuf"
		language "C++"

		includedirs
		{
			path.join(protobuf.settings.source, "src"),
		}
		files
		{
			path.join(protobuf.settings.source, "src/**.cc"),
		}
		removefiles
		{
			path.join(protobuf.settings.source, "src/**/*test.cc"),
			path.join(protobuf.settings.source, "src/google/protobuf/*test*.cc"),

			path.join(protobuf.settings.source, "src/google/protobuf/testing/**.cc"),
			path.join(protobuf.settings.source, "src/google/protobuf/compiler/**.cc"),

			path.join(protobuf.settings.source, "src/google/protobuf/arena_nc.cc"),
			path.join(protobuf.settings.source, "src/google/protobuf/util/internal/error_listener.cc"),
			path.join(protobuf.settings.source, "**/*_gcc.cc"),
		}

		-- dependencies
		zlib.import()

		-- not our code, ignore POSIX usage warnings for now
		defines { "_SCL_SECURE_NO_WARNINGS" }
		warnings "Off"

		-- always build as static lib, as we include our custom classes and therefore can't perform shared linking
		kind "StaticLib"
end
