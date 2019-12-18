workspace "Emscripten"
	location "build"
	architecture "x86"
    cppdialect "C++17"
    
	configurations { "Debug", "Release"}
	filter { "configurations:Debug" }
		symbols "On"
	filter { "configurations:Release" }
		optimize "On"
	filter { }

	targetdir ("build/bin/%{prj.name}/%{cfg.longname}")
	objdir ("build/obj/%{prj.name}/%{cfg.longname}")
    
project "Game"
	kind "WindowedApp"
	files "include/**"
	files "src/**"

    -- glm
    includedirs "thirdparty/glm/include"
        
	filter { "system:windows" }
        -- GLFW
        includedirs "thirdparty/GLFW/include"
        libdirs "thirdparty/GLFW/lib"
        links "glfw3"
        
        -- glew
        includedirs "thirdparty/glew/include"
        libdirs "thirdparty/glew/lib/Release/Win32"
        links "glew32"
        if os.target() == "windows" then
            postbuildcommands {
                "{COPY} %{wks.location}/../thirdparty/glew/bin/Release/Win32/glew32.dll %{cfg.targetdir}"
            }
        end
        
        -- OpenGL
		links { "OpenGL32" }
        
	filter { "system:not windows" }
        targetname "Game.html"
        if os.target() ~= "windows" then
            postbuildcommands {
                "{COPY} %{wks.location}/../start_pyth_server.sh %{cfg.targetdir}"
            }
        end
        
        -- Math
		links { "m" }
    
        -- OpenGL ES 2
		links { "GLESv2" }
    
        -- EGL
		links { "EGL" }
    
        -- X11
		links { "X11" }