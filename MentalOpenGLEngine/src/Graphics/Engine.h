#pragma once

#include <glad/glad.h>

struct GLFWwindow;

namespace Graphics
{
	class Engine
	{
	public:
		Engine(const int windowWidth, const int windowHeight, const char* title);

		Engine(const Engine& other) = delete;
		Engine& operator=(const Engine& other) = delete;

		virtual ~Engine();

		static inline Engine* GetInstance() { return mInstance; }

		bool Init();
		GLuint BuildShaderProgram();
		void BuildBuffers();
		void Run();

		virtual void OnResize(GLFWwindow* window, int width, int height);
		virtual void OnInput();
		virtual void OnRender();

	private:
		static Engine* mInstance;

		int mWindowWidth, mWindowHeight;
		const char* mTitle;

		GLFWwindow* mWindow;
		GLuint mShaderProgram;
		GLuint mVBO, mVAO, mEBO;
	};
}
