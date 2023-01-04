
function includeGameEngineFwk(enginePath)
	include(enginePath .. "GameEngineFwk_premake")
end

function rproject(name,enginePath,libPath)
	libPath = libPath or "./"
	-- default Pasta project
	project(name)
		
		debugdir "."
		dependson "GameEngineFwk"
		kind "StaticLib"
		language "C++"
		cppdialect "C++17"
		
		
		filter "action:vs*" -- Keep variables evaluated by VS so all paths are the same in VS ui
			targetdir "bin/$(Platform)/$(Configuration)"
		filter "action:not vs*"
			targetdir "bin/%{cfg.platform}/%{cfg.buildcfg}"
		
		filter {} -- reset filter states because premake is lua not python...
		
		if (usePrecompiledHeaders == nil) then
			usePrecompiledHeaders = true
		end
	
		if (usePrecompiledHeaders) then
			pchheader "stdafx.h"
			pchsource (libPath .. "/stdafx.cpp")
		end

		filter {}
		
		filter "files:**/Win32.cpp"
			flags {"NoPCH"}
		filter "files:**/NX.cpp"
			flags {"NoPCH"}
		filter {}
		--print( "here:" ..libPath )
		
		files {
			libPath,
			libPath .. "/**.h",
			libPath .. "/**.hpp",
			libPath .. "/**.cpp",
			
			-- libPath .. "/**.h",
			-- libPath .. "/**.inl"
		}

		filter "platforms:NX64"
			removefiles {
				"**/Win32.cpp"
			}
		filter "platforms:x64"
			removefiles {
				"**/NX.cpp"
			}
		filter {}
		
		includedirs {
			libPath,
			libPath .. "/../fmod/core/inc",
			libPath .. "/../fmod/studio/inc",
			libPath .. "/../sol",
			libPath .. "/../h264"
		}
		
		files {
			"**.natvis"
		}
		
		vpaths {
			["Natvis"] = { "**.natvis" },
		}
		defines { "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS" }
		
		defines { "BT_USE_SSE_IN_API","BT_USE_DOUBLE_PRECISION" }
		
		GameEngineFwk_defines(enginePath)
		GameEngineFwk_includedirs(enginePath)
		GameEngineFwk_configOptions(enginePath)
		
		stdWin()
		
		links {
			"GameEngineFwk",
		}
		
		
		
end
