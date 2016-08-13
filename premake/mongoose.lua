mongoose = {
	settings = nil,
}

function mongoose.setup(settings)
	if not settings.source then error("Missing source.") end

	mongoose.settings = settings
end

function mongoose.import()
	if not mongoose.settings then error("Run mongoose.setup first") end

	links { "mongoose" }
	mongoose.includes()
end

function mongoose.includes()
	if not mongoose.settings then error("Run mongoose.setup first") end

	includedirs { mongoose.settings.source }
end

function mongoose.project()
	if not mongoose.settings then error("Run mongoose.setup first") end

	project "mongoose"
		language "C"

		mongoose.includes()
		files
		{
			path.join(mongoose.settings.source, "*.c"),
			path.join(mongoose.settings.source, "*.h"),
		}

		-- not our code, ignore POSIX usage warnings for now
		warnings "Off"

		-- always build as static lib, as mongoose doesn't export anything
		kind "StaticLib"
end
