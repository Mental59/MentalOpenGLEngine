#include "Graphics/Engine.h"

int main()
{
	{
		Graphics::Engine engine(800, 600, "OpenGLEngine");

		if (!engine.Init())
		{
			return -1;
		}

		engine.Run();
	}

	return 0;
}
