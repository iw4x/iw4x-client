dxsdk = {
	source = path.join(dependencies.basePath, "dxsdk"),
}

function dxsdk.import()
	libdirs {path.join(dxsdk.source, "Lib/x86")}
	dxsdk.includes()
end

function dxsdk.includes()
	includedirs {
		path.join(dxsdk.source, "Include"),
	}
end

function dxsdk.project()
end

table.insert(dependencies, dxsdk)
