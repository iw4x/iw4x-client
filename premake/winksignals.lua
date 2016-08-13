winksignals = {
	settings = nil,
}

function winksignals.setup(settings)
	if not settings.source then error("Missing source.") end

	winksignals.settings = settings
end

function winksignals.import()
	if not winksignals.settings then error("Run winksignals.setup first") end

	winksignals.includes()
end

function winksignals.includes()
	if not winksignals.settings then error("Run winksignals.setup first") end

	includedirs { winksignals.settings.source }
end

function winksignals.project()
	if not winksignals.settings then error("Run winksignals.setup first") end

	-- Wink-Signals is header-only, so no project files needed for this
end
