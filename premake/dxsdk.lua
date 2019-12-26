dxsdk = {
	settings = nil
}

function dxsdk.setup(settings)
	if not settings.source then error("Missing source.") end

	dxsdk.settings = settings

	if not dxsdk.settings.defines then dxsdk.settings.defines = {} end
end

function dxsdk.import()
	if not dxsdk.settings then error("You need to call dxsdk.setup first") end

	--filter "platforms:*32"
		libdirs { path.join(dxsdk.settings.source, "Lib/x86") }
		
	--filter "platforms:*64"
	--	libdirs { path.join(dxsdk.settings.source, "Lib/x64") }
		
	--filter {}
	
	dxsdk.includes()
end

function dxsdk.includes()
	if not dxsdk.settings then error("You need to call dxsdk.setup first") end

	includedirs { path.join(dxsdk.settings.source, "Include") }
	defines(dxsdk.settings.defines)
end

function dxsdk.project()
end