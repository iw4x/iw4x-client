fonts = {
	source = path.join(dependencies.basePath, "extra/font"),
}

function fonts.import()
	fonts.includes()
end

function fonts.includes()
	includedirs {
		fonts.source,
	}
end

function fonts.project()
	project "fonts"
		language "C"

		fonts.includes()

		files {
			path.join(fonts.source, "Terminus_4.49.1.ttf.hpp"),
		}

		warnings "Off"
		kind "SharedItems"
end

table.insert(dependencies, fonts)
