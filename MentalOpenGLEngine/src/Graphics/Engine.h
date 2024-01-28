#pragma once

#include <vector>
#include <glad/glad.h>
#include "DataTypes.h"
#include "ShaderProgram.h"
#include "Camera.h"

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
		void BuildBuffers();
		void BuildTextures(
			ShaderProgram* shaderProgram,
			BuildTextureOptions optionList[],
			size_t n
		);
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
		ShaderProgram mLightCubeShaderProgram;
		GLuint mVBO, mCubeVAO, mEBO, mLightVAO;

		std::vector<GLuint> mTextureIDs;

		Camera mCamera;

		float mLastMouseXPos, mLastMouseYPos;
		bool mIsFirstMouseMove;
	};
}
