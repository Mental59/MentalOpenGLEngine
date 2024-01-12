#pragma once

#include <string>

struct GLFWwindow;

namespace Graphics
{
	class Engine
	{
	public:
		struct InitResult
		{
			bool isSuccessfull;
			std::string errorDescription;
		};

		Engine(const int windowWidth, const int windowHeight, const char* title);

		Engine(const Engine& other) = delete;
		Engine& operator=(const Engine& other) = delete;

		virtual ~Engine();

		static inline Engine* GetInstance() { return mInstance; }

		InitResult Init();
		void Run();

		virtual void OnResize(GLFWwindow* window, int width, int height);
		virtual void OnInput();
		virtual void OnRender();

	private:
		static Engine* mInstance;

		int mWindowWidth, mWindowHeight;
		const char* mTitle;

		GLFWwindow* mWindow;
	};
}
