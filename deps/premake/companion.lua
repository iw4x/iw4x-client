companion = {
  source = path.join(dependencies.basePath, "companion"),
}

function companion.import()
  companion.includes()
end

function companion.includes()
  includedirs {
    path.join(companion.source, "libcompanion"),
  }
end

function companion.project()

end

table.insert(dependencies, companion)
