nlohmannjson = {
	source = path.join(dependencies.basePath, "nlohmannjson"),
}

function nlohmannjson.import()
	nlohmannjson.includes()
end

function nlohmannjson.includes()
	includedirs {
		path.join(nlohmannjson.source, "single_include/nlohmann")
	}
end

function nlohmannjson.project()
end

table.insert(dependencies, nlohmannjson)
