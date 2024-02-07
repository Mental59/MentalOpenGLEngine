#include "Graphics/Engine.h"

int main()
{
	Graphics::Engine engine(1024, 768, "OpenGLEngine");

	if (!engine.Init(true, false))
	{
		return -1;
	}

	engine.Run();

	return 0;
}
