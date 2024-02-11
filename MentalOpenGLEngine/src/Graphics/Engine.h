#pragma once

#include <vector>
#include <glad/glad.h>
#include "CoreTypes.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Model.h"

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

		bool Init(bool vsync, bool windowedFullscreen);
		void Run();
		void UpdateTimer();

		virtual void OnResize(GLFWwindow* window, int width, int height);
		virtual void OnInput();
		virtual void OnRender();
		virtual void OnMouseMove(float xpos, float ypos);
		virtual void OnMouseScroll(float xOffset, float yOffset);

	private:
		static Engine* mInstance;

		int mWindowWidth, mWindowHeight;
		const char* mTitle;

		GLFWwindow* mWindow;
		ShaderProgram mBaseShaderProgram;

		Camera mCamera;
		Model mModel;

		float mLastMouseXPos, mLastMouseYPos;
		bool mIsFirstMouseMove;
	};
}
