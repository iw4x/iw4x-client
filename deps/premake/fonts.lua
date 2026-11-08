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
end

table.insert(dependencies, fonts)
