glm = {
	settings = nil,
}

function glm.setup(settings)
	if not settings.source then error("Missing source.") end

	glm.settings = settings
end

function glm.import()
	if not glm.settings then error("Run glm.setup first") end

	glm.includes()
end

function glm.includes()
	if not glm.settings then error("Run glm.setup first") end

	includedirs { glm.settings.source }
end

function glm.project()
	if not glm.settings then error("Run glm.setup first") end

end
