#include "Graphics/Engine.h"

int main()
{
#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Graphics::Engine engine(1920, 1080, "OpenGLEngine");

	bool vsync = false;
	bool windowedFullscreen = true;

	if (!engine.Init(vsync, windowedFullscreen))
	{
		return -1;
	}

	engine.Run();

	return 0;
}
