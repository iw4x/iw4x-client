mongoose = {
	source = path.join(dependencies.basePath, "mongoose"),
}

function mongoose.import()
	links "mongoose"

	mongoose.includes()
end

function mongoose.includes()
	includedirs {
		mongoose.source,
	}
end

function mongoose.project()
	project "mongoose"
		language "C"

		mongoose.includes()

		files {
			path.join(mongoose.source, "mongoose.c"),
			path.join(mongoose.source, "mongoose.h"),
		}

		warnings "Off"

		kind "StaticLib"
end

table.insert(dependencies, mongoose)
