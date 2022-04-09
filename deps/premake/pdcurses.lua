pdcurses = {
	source = path.join(dependencies.basePath, "pdcurses"),
}

function pdcurses.import()
	links {"pdcurses"}

	pdcurses.includes()
end

function pdcurses.includes()
	includedirs {pdcurses.source}
end

function pdcurses.project()
	project "pdcurses"
		language "C"

		pdcurses.includes()

		files
		{
			path.join(pdcurses.source, "pdcurses/*.c"),
			path.join(pdcurses.source, "pdcurses/*.h"),
			path.join(pdcurses.source, "wincon/*.c"),
			path.join(pdcurses.source, "wincon/*.h"),
		}

		warnings "Off"

		kind "StaticLib"
end

table.insert(dependencies, pdcurses)
