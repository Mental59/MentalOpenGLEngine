#include <iostream>
#include "Graphics/Engine.h"

int main()
{
	Graphics::Engine engine(800, 600, "OpenGLEngine");
	Graphics::Engine::InitResult initRes = engine.Init();

	if (!initRes.isSuccessfull)
	{
		std::cout << initRes.errorDescription << std::endl;
		return -1;
	}

	engine.Run();

	return 0;
}
