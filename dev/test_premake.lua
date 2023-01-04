-- project_premake.lua

-- Here is an example of the most common used configuration functions
-- For more in-depth use of premake, refer to the online documentation:
-- https://github.com/premake/premake-core/wiki

-- Add additional defines:
	-- defines { "DEFINE1", "DEFINE2" }

-- Remove existing defines:
	-- undefines { "DEFINE1", "DEFINE2" }

-- Add files to the project:
	-- files { "foo/**.cpp", "foo/**.h", "bar/file.lua" }

-- Remove files to the project:
	-- removefiles { "foo/**.h", "bar/file.lua" }

-- Add include directories:
	-- includedirs { "foo/", "bar/" }

-- Remove include directories:
	-- removeincludedirs { "foo/", "bar/" }
	
-- Link libraries:
	-- links { "libname", "lib2" }
	
-- Remove libraries:
	-- removelinks { "libname", "lib2" }
	
-- Add libraries directory
	-- libdirs { "foo/", "bar/" }
	
-- Remove libraries directory
	-- removelibdirs { "foo/", "bar/" }

-- Change this to match your engine's path
local enginePath = "../../pastagames/GameEngineFwk/"		
local rlibPath = "libs/r"

PASTA_AUDIO = false;

include(enginePath .. "premake/pastaproject")
include(rlibPath .. "/rlib")

function stdWin()
	filter {"system:windows", "action:vs*"}
			systemversion "latest"
			buildoptions {
				"/bigobj",
				"/Qpar",
				"/openmp",
				"/MP",
			}
	filter {}
end 

solution "App"
	defines { "" }
	platforms { "x64" }	--	Supported platforms (Win32, x64, ORBIS, Durango...)
	configurations { "Debug", "DebugOpt", "Release", "Final" }	--	Supported
	startproject "App"
	preferredtoolarchitecture "x86_64"
	rtti "on"
	
	-- This function creates a preconfigured working pasta project.
	-- By default, the project includes:
		-- Project/VisualStudio/*.c
		-- Project/VisualStudio/*.cpp
		-- Project/VisualStudio/*.h
		-- Classes/**.c
		-- Classes/**.cpp
		-- Classes/**.h
	pastaproject ("TestApp", enginePath)
		debugdir ".."
		cppdialect "C++17"
		dependson "R"
		dependson "bullet"
		dependson "h264"
		defines { "BT_USE_SSE_IN_API","BT_USE_DOUBLE_PRECISION" }
		--	Add Global additional configurations here
		icon "../res/icon.ico"
		
		links {
			"R",
			"bullet",
			"tb64",
			"h264",
		}
		
		files { 
			"../res/editor/**.lua",
			"../res/scripts/**.lua",
			"../res/scripts/**.chai",
			"../res/shaders/**.shl",
			"../res/shaders/**.glsl",
			"../res/shaders/pssl/**.pssl",
			"../res/shaders/glsl/**.glsl"
		}
		
		includedirs {
			"src",
			"libs/fmod/core/inc",
			"libs/fmod/studio/inc",
			"libs/r",
			"libs/bullet",
			"libs/utf8",
			"libs/sol",
			"libs/tb64",
			"libs/h264"
		}
		
		files {
			"src/**.h",
			"src/**.hpp",
			"src/**.cpp",
			
		}
		
		removefiles {
			"Project/VisualStudio/mainDT.cpp"
		}
		
		shaderdirs { "../res/" }
	
		stdWin()
			
		filter "platforms:Win32" 
			libdirs {
				"libs/fmod/core/lib/x86",
				"libs/fmod/studio/lib/x86",
			}
			links {
				"fmod_vc",
				"fmodstudio_vc"
			}
			postbuildcommands { 
				"rsync -ar --del --force ../../libs/fmod/core/lib/x86/fmod.dll \"$(OutDir)fmod.dll\" ",
				"rsync -ar --del --force ../../libs/fmod/studio/lib/x86/fmodstudio.dll \"$(OutDir)fmodstudio.dll\" " 
			}
		
		
		filter "platforms:x64"
			libdirs {
				"libs/fmod/core/lib/x64",
				"libs/fmod/studio/lib/x64",
			}
			links {
				"fmod_vc",
				"fmodstudio_vc"
			}
			
			postbuildcommands { 
				"rsync -ar --del --force ../../libs/fmod/core/lib/x64/fmod.dll \"$(OutDir)fmod.dll\" ",
				"rsync -ar --del --force ../../libs/fmod/studio/lib/x64/fmodstudio.dll \"$(OutDir)fmodstudio.dll\" " 
			}
			
		
		
		-- filter "platforms:ORBIS"
			--	Add PS4 additional configurations here
			
	project "GameEngineFwk"
		filter {"system:windows", "action:vs*"}
			systemversion "latest"
		--	Add Engine additional configurations here
		--	Warning: all paths should be relative to this project's directory
		
		-- Store the compiled lib inside the app folder, not the engine folder
		-- otherwise 2 projects using the same engine folder with different compilation options will overwrite the lib and keep breaking each other.
		filter "action:vs*" -- Keep variables evaluated by VS so all paths are the same in VS ui
			targetdir "bin/$(Platform)/$(Configuration)"
		filter "action:not vs*"
			targetdir "bin/%{cfg.platform}/%{cfg.buildcfg}"
		filter {}
		
	rproject("R",enginePath,rlibPath)
	
		filter {}
		
	project("tb64")
		debugdir "."
		kind "StaticLib"
		
		filter "configurations:Debug"
			optimize "Speed"
			runtime "Debug"
			
		filter "configurations:DebugOpt"
			optimize "Speed"
			runtime "Debug"

		filter "configurations:Release"
			runtime "Release"
			optimize "Speed"
			
		filter {}
		
		language "C++"
		cppdialect "C++17"
		filter "action:vs*" -- Keep variables evaluated by VS so all paths are the same in VS ui
			targetdir "bin/$(Platform)/$(Configuration)"
		filter "action:not vs*"
			targetdir "bin/%{cfg.platform}/%{cfg.buildcfg}"
		filter {}
		usePrecompiledHeaders = false
		files {
			"libs/tb64/**.h",
			"libs/tb64/**.c",
		}
		includedirs {
			"libs/tb64"
		}
		defines { "NO_AVX" }
		defines { "NO_AVX2" }
		
		stdWin()
		
	project("h264")
		debugdir "."
		kind "StaticLib"
		
		filter "configurations:Debug"
			optimize "Speed"
			runtime "Debug"
			
		filter "configurations:DebugOpt"
			optimize "Speed"
			runtime "Debug"

		filter "configurations:Release"
			runtime "Release"
			optimize "Speed"
			
		filter {}
		
		language "C++"
		cppdialect "C++17"
		filter "action:vs*" -- Keep variables evaluated by VS so all paths are the same in VS ui
			targetdir "bin/$(Platform)/$(Configuration)"
		filter "action:not vs*"
			targetdir "bin/%{cfg.platform}/%{cfg.buildcfg}"
		filter {}
		usePrecompiledHeaders = false
		files {
			"libs/h264/**.h",
			"libs/h264/**.c",
		}
		includedirs {
			"libs/h264"
		}
		stdWin()
		
	project("bullet")
		debugdir "."
		kind "StaticLib"
		
		filter "configurations:Debug"
			optimize "Speed"
			runtime "Debug"
			
		filter "configurations:DebugOpt"
			optimize "Speed"
			runtime "Debug"

		filter "configurations:Release"
			runtime "Release"
			optimize "Speed"
			
		filter {}
		
		language "C++"
		cppdialect "C++14"
		filter "action:vs*" -- Keep variables evaluated by VS so all paths are the same in VS ui
			targetdir "bin/$(Platform)/$(Configuration)"
		filter "action:not vs*"
			targetdir "bin/%{cfg.platform}/%{cfg.buildcfg}"
		filter {}
		usePrecompiledHeaders = false
		files {
			"libs/bullet/**.h",
			"libs/bullet/**.cpp",
		}
		includedirs {
			"libs/bullet"
		}
		stdWin()
		filter {"system:windows"}
			vectorextensions "SSE3"
			defines { "BT_USE_SSE_IN_API","BT_USE_DOUBLE_PRECISION" }
			disablewarnings { 
				"4006",
				"4221",
				"4244",
				"4267",
				"4305",
			}
		filter {}
		
		
		
		
		
		
		
		