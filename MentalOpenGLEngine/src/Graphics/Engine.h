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
#include "CubeMap.h"
#include "DepthMap.h"
#include "GFrameBuffer.h"

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
		void Update();
		void UpdateTimer();

		virtual void OnResize(GLFWwindow* window, int width, int height);
		virtual void OnInput();
		virtual void OnRender();
		virtual void OnMouseMove(float xpos, float ypos);
		virtual void OnMouseScroll(float xOffset, float yOffset);

	private:
		void DrawScene(ShaderProgram& shader);
		void SetupScene(const glm::mat4& view, const glm::mat4& projection);
		void ShadowPass();
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
		unsigned int LoadTexture(const char* path, bool flip = false, bool srgb = false);

		static Engine* mInstance;

		int mWindowWidth, mWindowHeight;
		float mAspectRatio;
		const char* mTitle;

		GLFWwindow* mWindow;

		ShaderProgram mBaseShaderProgram;
		ShaderProgram mBaseInstancedShaderProgram;
		ShaderProgram mOutlineShaderProgram;
		ShaderProgram mFramebufferScreenShaderProgram;
		ShaderProgram mSkyboxShaderProgram;
		ShaderProgram mEnvironmentMappingShaderProgram;
		ShaderProgram mNormalsVisualizationShaderProgram;
		ShaderProgram mLightSourceShaderProgram;
		ShaderProgram mDirectionalShadowMappingShaderProgram;
		ShaderProgram mPointShadowMappingShaderProgram;
		ShaderProgram mGaussianBlurShaderProgram;

		ShaderProgram mGBufferShaderProgram;
		ShaderProgram mDeferredShaderProgram;

		FrameBuffer mFrameBuffer;
		FrameBuffer mRearViewFrameBuffer;
		GFrameBuffer mGFrameBuffer;

		ScreenQuad mScreenQuad;
		CubeMap mCubemap;
		DepthMap mDirectionalDepthMap;
		DepthMap mPointDepthMap;

		Camera mCamera;
		std::unordered_map<std::string, unsigned int> mLoadedTextures;

		Core::Texture mDefaultTexture;

		float mLastMouseXPos, mLastMouseYPos;
		bool mIsFirstMouseMove;

		unsigned int mUBOMatrices;

		unsigned int mPingPongFrameBuffers[2];
		unsigned int mPingPongColorBuffers[2];
	};
}
