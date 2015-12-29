solution "iw4x"
	location ("./build")
	configurations { "Normal" }

	project "iw4x"
		kind "SharedLib"
		language "C++"
		files { "src/**.hpp", "src/**.cpp" }
		--toolset "v120" -- Compatibility for users

		configuration "Normal"
			defines { "NDEBUG" }
			flags { "Optimize", "MultiProcessorCompile", "Symbols" }
