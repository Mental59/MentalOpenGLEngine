#pragma once

#include <vector>
#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include "CoreTypes.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Model.h"
#include "FrameBuffer.h"
#include "Primitives/ScreenQuad.h"

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
		void DrawScene(const glm::mat4& view);
		void ImportModels(
			const std::vector<Core::ModelImport>& imports,
			std::vector<std::shared_ptr<Model>>* models
		);
		void DrawModels(
			const std::vector<std::shared_ptr<Model>>& models,
			ShaderProgram& shader,
			bool outline = false
		);
		void DrawTransparentModels(
			const std::vector<std::shared_ptr<Model>>& models,
			ShaderProgram& shader
		);

		static Engine* mInstance;

		int mWindowWidth, mWindowHeight;
		const char* mTitle;

		GLFWwindow* mWindow;

		ShaderProgram mBaseShaderProgram;
		ShaderProgram mOutlineShaderProgram;
		ShaderProgram mFramebufferScreenShaderProgram;

		FrameBuffer mFrameBuffer;
		FrameBuffer mRearViewFrameBuffer;
		ScreenQuad mScreenQuad;

		Camera mCamera;
		std::unordered_map<std::string, unsigned int> mLoadedTextures;

		Core::Texture mDefaultTexture;

		float mLastMouseXPos, mLastMouseYPos;
		bool mIsFirstMouseMove;
	};
}
