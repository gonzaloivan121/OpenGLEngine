#pragma once

#include "Core/Profiler.h"

extern Application* CreateApplication();

int Main(int argc, char** argv) {
	OGE_PROFILE_BEGIN_SESSION("Startup", "profile_startup.json");
	Application* app = CreateApplication();
	OGE_PROFILE_END_SESSION();

	OGE_PROFILE_BEGIN_SESSION("Runtime", "profile_runtime.json");
	app->Run();
	OGE_PROFILE_END_SESSION();

	OGE_PROFILE_BEGIN_SESSION("Shutdown", "profile_shutdown.json");
	delete app;
	OGE_PROFILE_END_SESSION();

	return 0;
}

#if defined(OGE_PLATFORM_WINDOWS) && defined(OGE_DIST)

#include <Windows.h>

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow) {
	return Main(__argc, __argv);
}

#else

int main(int argc, char** argv) {
	return Main(argc, argv);
}

#endif // defined(OGE_PLATFORM_WINDOWS) && defined(OGE_DIST)