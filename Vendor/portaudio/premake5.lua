project "portaudio"
    kind "StaticLib"
    language "C"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    includedirs {
        "include",
        "src/common"
    }

    files {
        "src/common/pa_allocation.c",
        "src/common/pa_converters.c",
        "src/common/pa_cpuload.c",
        "src/common/pa_debugprint.c",
        "src/common/pa_dither.c",
        "src/common/pa_front.c",
        "src/common/pa_process.c",
        "src/common/pa_ringbuffer.c",
        "src/common/pa_stream.c",
        "src/common/pa_trace.c"
    }

    filter "system:windows"
        systemversion "latest"
        defines {
            "PA_USE_WASAPI=1",
            "PAWIN_USE_WDMKS_DEVICE_INFO=1",
            "_CRT_SECURE_NO_WARNINGS"
        }
        includedirs {
            "src/os/win"
        }
        files {
            "src/hostapi/wasapi/pa_win_wasapi.c",
            "src/os/win/pa_win_hostapis.c",
            "src/os/win/pa_win_util.c",
            "src/os/win/pa_win_waveformat.c",
            "src/os/win/pa_win_wdmks_utils.c",
            "src/os/win/pa_win_version.c",
            "src/os/win/pa_win_coinitialize.c"
        }
        links {
            "ole32",
            "uuid"
        }

    filter "system:linux"
        pic "on"
		systemversion "latest"
        defines {
            "PA_USE_ALSA=1"
        }
        includedirs {
            "src/os/unix"
        }
        files { 
            "src/hostapi/alsa/pa_linux_alsa.c",
            "src/os/unix/pa_unix_hostapis.c",
            "src/os/unix/pa_unix_util.c"
        }
        links {
            "asound",
            "pthread"
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
		runtime "Release"
		optimize "on"
		symbols "off"