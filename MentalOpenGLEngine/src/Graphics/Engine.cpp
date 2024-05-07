#include "Engine.h"

#include <iostream>
#include <format>
#include <glad/glad.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include "Shader.h"
#include "Camera.h"
#include "Time.h"
#include "Utils.h"

Graphics::Engine* Graphics::Engine::mInstance(nullptr);

Model ASTEROID_MODEL;
Model MARS_MODEL;
Model SPHERE_MODEL;
Model CUBE_MODEL;
Model FLOOR_MODEL;
Model NANOSUIT_MODEL;
Model SPONZA_MODEL;
Model BACKPACK_MODEL(true);

glm::vec3 LIGHT_DIRECTION = glm::normalize(glm::vec3(1.0f, -0.5f, 1.0f));
glm::mat4 DIR_LIGHT_SPACE_MAT;
glm::vec3 DIR_LIGHT_POS;

glm::vec3 POINT_LIGHT_POSITIONS[4]{
		glm::vec3(0.0f, 0.5f, 1.5f),
		glm::vec3(-4.0f, 0.5f, -3.0f),
		glm::vec3(3.0f, 0.5f, 1.0f),
		glm::vec3(-1.2f, 0.4f, -1.0f)
};

glm::vec3 POINT_LIGHT_COLORS[4]{
	glm::vec3(5.0f, 5.0f, 5.0f),
	glm::vec3(10.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 0.0f, 15.0f),
	glm::vec3(0.0f, 5.0f, 0.0f)
};

//constexpr int ASTEROIDS_NUM = 100000;
//glm::mat4 ASTEROID_TRANSFORMS[ASTEROIDS_NUM];

Graphics::Engine::Engine(const int windowWidth, const int windowHeight, const char* title) :
	mWindowWidth(windowWidth),
	mWindowHeight(windowHeight),
	mAspectRatio(static_cast<float>(windowWidth) / static_cast<float>(windowHeight)),
	mTitle(title),
	mWindow(nullptr),
	mBaseShaderProgram(),
	mCamera(glm::vec3(0.0f, 0.0f, 0.0f), 5.0f, 0.1f),
	mLastMouseXPos(0.0f), mLastMouseYPos(0.0f), mIsFirstMouseMove(true),
	mDefaultTexture{},
	mUBOMatrices(0u)
{
	mInstance = this;
}

Graphics::Engine::~Engine()
{
	for (const auto& loadedTexture : mLoadedTextures)
	{
		glDeleteTextures(1, &loadedTexture.second);
	}

	glDeleteBuffers(1, &mUBOMatrices);

	glDeleteFramebuffers(2, mPingPongFrameBuffers);
	glDeleteTextures(2, mPingPongColorBuffers);

	glfwTerminate();
}

static void OnResizeCallback(GLFWwindow* window, int width, int height)
{
	Graphics::Engine::GetInstance()->OnResize(window, width, height);
}

static void OnCursorPoseCallback(GLFWwindow* window, double xpos, double ypos)
{
	Graphics::Engine::GetInstance()->OnMouseMove(static_cast<float>(xpos), static_cast<float>(ypos));
}

static void OnMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	Graphics::Engine::GetInstance()->OnMouseScroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

bool Graphics::Engine::Init(bool vsync, bool windowedFullscreen)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_SAMPLES, 4);

	if (windowedFullscreen)
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

		mWindowWidth = mode->width;
		mWindowHeight = mode->height;

		mWindow = glfwCreateWindow(mode->width, mode->height, mTitle, monitor, NULL);
	}
	else
	{
		mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, mTitle, NULL, NULL);
	}

	if (mWindow == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		return false;
	}

	glfwMakeContextCurrent(mWindow);
	glfwSwapInterval(vsync ? 1 : 0);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;

	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &nrAttributes);
	std::cout << "Maximum nr of vertex uniform components supported: " << nrAttributes << std::endl;

	const GLubyte* glVersion = glGetString(GL_VERSION);
	std::cout << "OpenGL version: " << glVersion << std::endl;

	glfwSetFramebufferSizeCallback(mWindow, OnResizeCallback);
	glfwSetCursorPosCallback(mWindow, OnCursorPoseCallback);
	glfwSetScrollCallback(mWindow, OnMouseScrollCallback);

	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	Shader baseVertexShader("src/Shaders/base.vert", Shader::Vertex);
	Shader baseInstancedVertexShader("src/Shaders/baseInstanced.vert", Shader::Vertex);
	Shader baseFragmentShader("src/Shaders/base.frag", Shader::Fragment);
	Shader outlineFragmentShader("src/Shaders/outline.frag", Shader::Fragment);
	Shader environmentMappingFragmentShader("src/Shaders/environmentMapping.frag", Shader::Fragment);
	Shader cubemapVertexShader("src/Shaders/cubemap.vert", Shader::Vertex);
	Shader cubemapFragmentShader("src/Shaders/cubemap.frag", Shader::Fragment);
	Shader framebufferVertexShader("src/Shaders/framebufferScreen.vert", Shader::Vertex);
	Shader framebufferFragmentShader("src/Shaders/framebufferScreen.frag", Shader::Fragment);
	Shader explodeGeometryShader("src/Shaders/explode.geom", Shader::Geometry);
	Shader normalsVisualizationVertexShader("src/Shaders/normalsVisualization.vert", Shader::Vertex);
	Shader normalsVisualizationFragmentShader("src/Shaders/normalsVisualization.frag", Shader::Fragment);
	Shader normalsVisualizationGeometryShader("src/Shaders/normalsVisualization.geom", Shader::Geometry);
	Shader lightSourceVertexShader("src/Shaders/lightSource.vert", Shader::Vertex);
	Shader lightSourceFragmentShader("src/Shaders/lightSource.frag", Shader::Fragment);
	Shader dirShadowMappingVertexShader("src/Shaders/directionalShadowMapping.vert", Shader::Vertex);
	Shader dirShadowMappingFragmentShader("src/Shaders/directionalShadowMapping.frag", Shader::Fragment);
	Shader pointShadowMappingVertShader("src/Shaders/pointShadowMapping.vert", Shader::Vertex);
	Shader pointShadowMappingFragShader("src/Shaders/pointShadowMapping.frag", Shader::Fragment);
	Shader pointShadowMappingGeomShader("src/Shaders/pointShadowMapping.geom", Shader::Geometry);

	Shader gaussianBlurVertShader("src/Shaders/gaussianBlur.vert", Shader::Vertex);
	Shader gaussianBlurFragShader("src/Shaders/gaussianBlur.frag", Shader::Fragment);

	Shader gBufferVertShader("src/Shaders/gBuffer.vert", Shader::Vertex);
	Shader gBufferFragShader("src/Shaders/gBuffer.frag", Shader::Fragment);

	Shader deferredVertShader("src/Shaders/deferred.vert", Shader::Vertex);
	Shader deferredFragShader("src/Shaders/deferred.frag", Shader::Fragment);

	mBaseShaderProgram.Build({ baseVertexShader, baseFragmentShader });
	//mBaseInstancedShaderProgram.Build({ baseInstancedVertexShader, baseFragmentShader });
	mOutlineShaderProgram.Build({ baseVertexShader, outlineFragmentShader });
	mEnvironmentMappingShaderProgram.Build({ baseVertexShader, environmentMappingFragmentShader });
	mSkyboxShaderProgram.Build({ cubemapVertexShader, cubemapFragmentShader });
	mFramebufferScreenShaderProgram.Build({ framebufferVertexShader, framebufferFragmentShader });
	mNormalsVisualizationShaderProgram.Build({ normalsVisualizationVertexShader, normalsVisualizationFragmentShader, normalsVisualizationGeometryShader });
	mLightSourceShaderProgram.Build({ lightSourceFragmentShader, lightSourceVertexShader });
	mDirectionalShadowMappingShaderProgram.Build({ dirShadowMappingFragmentShader, dirShadowMappingVertexShader });
	mPointShadowMappingShaderProgram.Build({ pointShadowMappingVertShader, pointShadowMappingFragShader, pointShadowMappingGeomShader });
	mGaussianBlurShaderProgram.Build({ gaussianBlurVertShader, gaussianBlurFragShader });
	mGBufferShaderProgram.Build({ gBufferVertShader, gBufferFragShader });
	mDeferredShaderProgram.Build({ deferredVertShader,deferredFragShader });

	// Setting texture units
	mFramebufferScreenShaderProgram.Bind();
	mFramebufferScreenShaderProgram.SetUniform1i("uScreenTexture", 0);
	mFramebufferScreenShaderProgram.Unbind();
	mSkyboxShaderProgram.Bind();
	mSkyboxShaderProgram.SetUniform1i("uSkybox", 0);
	mSkyboxShaderProgram.Unbind();
	mEnvironmentMappingShaderProgram.Bind();
	mEnvironmentMappingShaderProgram.SetUniform1i("uSkybox", 0);
	mEnvironmentMappingShaderProgram.Unbind();
	mBaseShaderProgram.Bind();
	mBaseShaderProgram.SetUniform1i("uShadowMap", 16);
	mBaseShaderProgram.SetUniform1i("uShadowCubeMap", 17);
	mBaseShaderProgram.Unbind();
	mGaussianBlurShaderProgram.Bind();
	mGaussianBlurShaderProgram.SetUniform1i("uImage", 0);
	mGaussianBlurShaderProgram.Unbind();
	mDeferredShaderProgram.Bind();
	mDeferredShaderProgram.SetUniform1i("gPosition", 0);
	mDeferredShaderProgram.SetUniform1i("gNormal", 1);
	mDeferredShaderProgram.SetUniform1i("gAlbedoSpec", 2);
	mDeferredShaderProgram.SetUniform1i("uDirLights[0].shadowMap", 3);
	mDeferredShaderProgram.SetUniform1i("uNumPointLights[0].shadowCubeMap", 4);
	mDeferredShaderProgram.Unbind();

	//Setting uniform block bindings
	unsigned int uniformMatricesBlockBinding = 0;
	mBaseShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);
	mBaseInstancedShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);
	mOutlineShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);
	mEnvironmentMappingShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);
	mLightSourceShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);
	mGBufferShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);

	size_t bufferSize = 2 * sizeof(glm::mat4);
	glGenBuffers(1, &mUBOMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER, mUBOMatrices);
	glBufferData(GL_UNIFORM_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, uniformMatricesBlockBinding, mUBOMatrices, 0, bufferSize);

	mDirectionalDepthMap.Build(2048, 2048, DepthMap::Directional);
	mPointDepthMap.Build(2048, 2048, DepthMap::Point);

	//constexpr unsigned int uniformsNum = 6;
	//const char* uniformNames[uniformsNum]{
	//	"uField1", "uField2", "uField3", "uField4", "uField5", "uField6"
	//};
	//unsigned int uniformIndices[uniformsNum];
	//int uniformOffsets[uniformsNum];
	//glGetUniformIndices(mBaseShaderProgram.GetID(), uniformsNum, uniformNames, uniformIndices);
	//glGetActiveUniformsiv(mBaseShaderProgram.GetID(), uniformsNum, uniformIndices, GL_UNIFORM_OFFSET, uniformOffsets);
	//for (int i = 0; i < uniformsNum; i++)
	//{
	//	std::cout << std::format("Uniform Info: name={}; index={}; offset={}\n", uniformNames[i], uniformIndices[i], uniformOffsets[i]);
	//}

	// Load default diffuse texture
	unsigned int defaultDiffuseTextureId = GLLoadTextureFromFile("resources/textures/default.png", false, true);
	mLoadedTextures["resources/textures/default.png"] = defaultDiffuseTextureId;
	mDefaultTexture = { defaultDiffuseTextureId, Core::Diffuse };

	int samples = 8;
	//mFrameBuffer.Create(mWindowWidth, mWindowHeight, samples);
	mGFrameBuffer.Create(mWindowWidth, mWindowHeight);
	//mRearViewFrameBuffer.Create(mWindowWidth, mWindowHeight);
	mScreenQuad.Create();

	// Pingpong fbos for blurring
	glGenFramebuffers(2, mPingPongFrameBuffers);
	glGenTextures(2, mPingPongColorBuffers);
	for (int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mPingPongFrameBuffers[i]);
		glBindTexture(GL_TEXTURE_2D, mPingPongColorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mWindowWidth, mWindowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mPingPongColorBuffers[i], 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Ping Pong Framebuffer is not complete!" << std::endl;
	}

	//unsigned int rockTextureId = GLLoadTextureFromFile("resources/objects/rock/rock.png");
	//mLoadedTextures["resources/objects/rock/rock.png"] = rockTextureId;
	//ASTEROID_MODEL.SetDefaultTexture({ rockTextureId, Core::Diffuse });
	//ASTEROID_MODEL.Load("resources/objects/rock/rock.obj");
	//MARS_MODEL.Load("resources/objects/planet/planet.obj");
	SPHERE_MODEL.Load("resources/objects/sphere/sphere.obj");

	CUBE_MODEL.SetDefaultTexture({ LoadTexture("resources/textures/container2.png", false, true), Core::Diffuse });
	CUBE_MODEL.SetDefaultTexture({ LoadTexture("resources/textures/container2_specular.png"), Core::Specular });
	CUBE_MODEL.Load("resources/objects/cube/cube.obj");

	FLOOR_MODEL.SetDefaultTexture({ LoadTexture("resources/textures/bricks2.jpg", false, true), Core::Diffuse });
	FLOOR_MODEL.SetDefaultTexture({ LoadTexture("resources/textures/bricks2_normal.jpg", false, false), Core::Normal });
	FLOOR_MODEL.SetDefaultTexture({ LoadTexture("resources/textures/bricks2_disp.jpg", false, false), Core::Height });
	FLOOR_MODEL.Load("resources/objects/cube/cube.obj");

	NANOSUIT_MODEL.Load("resources/objects/nanosuit/nanosuit.obj");
	BACKPACK_MODEL.Load("resources/objects/backpack/backpack.obj");

	//SPONZA_MODEL.Load("resources/objects/sponza/sponza.obj");

	//const char* faces[6]{
	//	"resources/skyboxes/SpaceLightblue/right.png",
	//	"resources/skyboxes/SpaceLightblue/left.png",
	//	"resources/skyboxes/SpaceLightblue/top.png",
	//	"resources/skyboxes/SpaceLightblue/bottom.png",
	//	"resources/skyboxes/SpaceLightblue/front.png",
	//	"resources/skyboxes/SpaceLightblue/back.png"
	//};
	//mCubemap.Load(faces);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe mode

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);
	glDisable(GL_BLEND);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glEnable(GL_MULTISAMPLE);
	//glEnable(GL_FRAMEBUFFER_SRGB);

	//srand(glfwGetTime());
	//float radius = 100.0;
	//float offset = 25.0f;
	//for (int i = 0; i < ASTEROIDS_NUM; i++)
	//{
	//	glm::mat4 model = glm::mat4(1.0f);

	//	// 1. translation: displace along circle with 'radius' in range [-offset, offset]
	//	float angle = (float)i / (float)ASTEROIDS_NUM * 360.0f;
	//	float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
	//	float x = sin(angle) * radius + displacement;
	//	displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
	//	float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
	//	displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
	//	float z = cos(angle) * radius + displacement;
	//	model = glm::translate(model, glm::vec3(x, y, z));

	//	// 2. scale: scale between 0.05 and 0.25f
	//	float scale = (rand() % 20) / 100.0f + 0.05;
	//	model = glm::scale(model, glm::vec3(scale));

	//	// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
	//	float rotAngle = (rand() % 360);
	//	model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

	//	ASTEROID_TRANSFORMS[i] = model;
	//}

	//ASTEROID_MODEL.SetupInstancedDrawing(ASTEROID_TRANSFORMS, ASTEROIDS_NUM, 3);

	return true;
}

void Graphics::Engine::Run()
{
	while (!glfwWindowShouldClose(mWindow))
	{
		OnInput();
		UpdateTimer();
		Update();
		OnRender();
		glfwPollEvents();
	}
}

void Graphics::Engine::Update()
{
	//POINT_LIGHT_POS.x = sin(Time::LastFrame) * 4.0f;
	//POINT_LIGHT_POS.z = cos(Time::LastFrame) * 4.0f;
}

void Graphics::Engine::UpdateTimer()
{
	float currentFrame = static_cast<float>(glfwGetTime());
	Time::DeltaTime = currentFrame - Time::LastFrame;
	Time::LastFrame = currentFrame;
}

void Graphics::Engine::OnResize(GLFWwindow* window, int width, int height)
{
	mWindowWidth = width;
	mWindowHeight = height;
	mAspectRatio = static_cast<float>(width) / static_cast<float>(height);

	glViewport(0, 0, width, height);
}

void Graphics::Engine::OnInput()
{
	if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(mWindow, true);
	}

	if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		mCamera.Move(Camera::Movement::Left);
	}
	if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		mCamera.Move(Camera::Movement::Right);
	}
	if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		mCamera.Move(Camera::Movement::Forward);
	}
	if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		mCamera.Move(Camera::Movement::Backward);
	}
	if (glfwGetKey(mWindow, GLFW_KEY_Q) == GLFW_PRESS)
	{
		mCamera.Move(Camera::Movement::Down);
	}
	if (glfwGetKey(mWindow, GLFW_KEY_E) == GLFW_PRESS)
	{
		mCamera.Move(Camera::Movement::Up);
	}
}

void Graphics::Engine::OnRender()
{
	//Render to depth map
	ShadowPass();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, mWindowWidth, mWindowHeight);

	//Drawing scene
	mGFrameBuffer.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	SetupScene(mCamera.GetViewMatrix(), mCamera.GetProjectionMatrix(mAspectRatio));
	// geometry pass
	DrawScene(mGBufferShaderProgram);


	////Blurring bright fragments with two-pass Gaussian Blur 
	//mGaussianBlurShaderProgram.Bind();
	//mGaussianBlurShaderProgram.SetUniformVec2("uSampleDistance", glm::value_ptr(glm::vec2(2.0f, 2.0f)));
	//bool isHorizontal = true, isFirstIteration = true;
	//unsigned int numPasses = 5;
	//glDisable(GL_DEPTH_TEST);
	//for (unsigned int i = 0; i < numPasses * 2; i++)
	//{
	//	mGaussianBlurShaderProgram.SetUniform1i("uHorizontal", isHorizontal);
	//	glBindFramebuffer(GL_FRAMEBUFFER, mPingPongFrameBuffers[isHorizontal]);
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, isFirstIteration ? mFrameBuffer.GetBrightTextureColorId() : mPingPongColorBuffers[!isHorizontal]);
	//	mScreenQuad.Draw();
	//	isHorizontal = !isHorizontal;
	//	isFirstIteration = false;
	//}
	//glEnable(GL_DEPTH_TEST);


	//Deferred calculations
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mDeferredShaderProgram.Bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mGFrameBuffer.GetPositionTextureId());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mGFrameBuffer.GetNormalTextureId());
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mGFrameBuffer.GetAlbedoSpecularTextureId());
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, mDirectionalDepthMap.GetTextureColorId());
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, mPointDepthMap.GetTextureColorId());

	glDisable(GL_DEPTH_TEST);
	mScreenQuad.Draw();
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, mGFrameBuffer.GetFrameBufferId());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, mWindowWidth, mWindowHeight, 0, 0, mWindowWidth, mWindowHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	mLightSourceShaderProgram.Bind();
	for (int i = 0; i < 4; i++)
	{
		mLightSourceShaderProgram.SetUniformVec3("uLightColor", glm::value_ptr(POINT_LIGHT_COLORS[i]));
		glm::mat4 lightSourceMat = glm::mat4(1.0f);
		lightSourceMat = glm::translate(lightSourceMat, POINT_LIGHT_POSITIONS[i]);
		lightSourceMat = glm::scale(lightSourceMat, glm::vec3(0.15f));
		SPHERE_MODEL.Draw(mLightSourceShaderProgram, lightSourceMat);
	}

	glfwSwapBuffers(mWindow);
}

void Graphics::Engine::ShadowPass()
{
	float nearPlane = 0.1f, farPlane = 100.0f;

	glm::mat4 dirLightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, nearPlane, farPlane);
	DIR_LIGHT_POS = -LIGHT_DIRECTION * 30.0f;
	glm::mat4 dirLightView = glm::lookAt(
		DIR_LIGHT_POS,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	DIR_LIGHT_SPACE_MAT = dirLightProjection * dirLightView;

	float shadowAspect = static_cast<float>(mPointDepthMap.GetWidth()) / static_cast<float>(mPointDepthMap.GetHeight());
	glm::mat4 pointLightProjection = glm::perspective(glm::radians(90.0f), shadowAspect, nearPlane, farPlane);
	glm::mat4 pointShadowTransforms[6];
	pointShadowTransforms[0] = pointLightProjection * glm::lookAt(POINT_LIGHT_POSITIONS[0], POINT_LIGHT_POSITIONS[0] + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // right
	pointShadowTransforms[1] = pointLightProjection * glm::lookAt(POINT_LIGHT_POSITIONS[0], POINT_LIGHT_POSITIONS[0] + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // left
	pointShadowTransforms[2] = pointLightProjection * glm::lookAt(POINT_LIGHT_POSITIONS[0], POINT_LIGHT_POSITIONS[0] + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // top
	pointShadowTransforms[3] = pointLightProjection * glm::lookAt(POINT_LIGHT_POSITIONS[0], POINT_LIGHT_POSITIONS[0] + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)); // bottom
	pointShadowTransforms[4] = pointLightProjection * glm::lookAt(POINT_LIGHT_POSITIONS[0], POINT_LIGHT_POSITIONS[0] + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // back
	pointShadowTransforms[5] = pointLightProjection * glm::lookAt(POINT_LIGHT_POSITIONS[0], POINT_LIGHT_POSITIONS[0] + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // forward

	mDirectionalShadowMappingShaderProgram.Bind();
	mDirectionalShadowMappingShaderProgram.SetUniformMat4("uLightSpaceMatrix", glm::value_ptr(DIR_LIGHT_SPACE_MAT));

	mPointShadowMappingShaderProgram.Bind();
	for (unsigned int i = 0; i < 6; i++)
	{
		mPointShadowMappingShaderProgram.SetUniformMat4(
			std::format("uShadowMatrices[{}]", i), glm::value_ptr(pointShadowTransforms[i])
		);
	}
	mPointShadowMappingShaderProgram.SetUniformVec3("uLightPos", glm::value_ptr(POINT_LIGHT_POSITIONS[0]));
	mPointShadowMappingShaderProgram.SetUniform1f("uFarPlane", farPlane);

	glViewport(0, 0, mDirectionalDepthMap.GetWidth(), mDirectionalDepthMap.GetHeight());
	mDirectionalDepthMap.Bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	DrawScene(mDirectionalShadowMappingShaderProgram);

	glViewport(0, 0, mPointDepthMap.GetWidth(), mPointDepthMap.GetHeight());
	mPointDepthMap.Bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	DrawScene(mPointShadowMappingShaderProgram);
}

void Graphics::Engine::DrawScene(ShaderProgram& shader)
{
	shader.Bind();

	static constinit glm::vec3 backpackPositions[]{
		glm::vec3(-3.0, -0.5, -3.0),
		glm::vec3(0.0, -0.5, -3.0),
		glm::vec3(3.0, -0.5, -3.0),
		glm::vec3(-6.0, -0.5, -3.0),
		glm::vec3(6.0, -0.5, -3.0),

		glm::vec3(-3.0, -0.5, 0.0),
		glm::vec3(0.0, -0.5, 0.0),
		glm::vec3(3.0, -0.5, 0.0),
		glm::vec3(-6.0, -0.5, 0.0),
		glm::vec3(6.0, -0.5, 0.0),

		glm::vec3(-3.0, -0.5, 3.0),
		glm::vec3(0.0, -0.5, 3.0),
		glm::vec3(3.0, -0.5, 3.0),
		glm::vec3(-6.0, -0.5, 3.0),
		glm::vec3(6.0, -0.5, 3.0),

		glm::vec3(-3.0, -0.5, -6.0),
		glm::vec3(0.0, -0.5, -6.0),
		glm::vec3(3.0, -0.5, -6.0),
		glm::vec3(-6.0, -0.5, -6.0),
		glm::vec3(6.0, -0.5, -6.0),

		glm::vec3(-3.0, -0.5, 6.0),
		glm::vec3(0.0, -0.5, 6.0),
		glm::vec3(3.0, -0.5, 6.0),
		glm::vec3(-6.0, -0.5, 6.0),
		glm::vec3(6.0, -0.5, 6.0),
	};

	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0));
	model = glm::scale(model, glm::vec3(12.5f, 0.5f, 12.5f));
	shader.SetUniform1f("uTexTiling", 4.0f);
	FLOOR_MODEL.Draw(shader, model);
	shader.SetUniform1f("uTexTiling", 1.0f);

	for (const glm::vec3& backpackPos : backpackPositions)
	{
		model = glm::translate(glm::mat4(1.0f), backpackPos);
		model = glm::scale(model, glm::vec3(0.5f));
		BACKPACK_MODEL.Draw(shader, model);
	}
}

void Graphics::Engine::SetupScene(
	const glm::mat4& view,
	const glm::mat4& projection
)
{
	static glm::vec3 lightColor(0.9, 0.68, 0.24);
	static glm::vec3 ambientColor = glm::vec3(0.1f, 0.1f, 0.1f) * lightColor;
	static glm::vec3 diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f) * lightColor;
	static glm::vec3 specularColor = glm::vec3(1.0f, 1.0f, 1.0f) * lightColor;

	static float shininess = 256.0f;

	glBindBuffer(GL_UNIFORM_BUFFER, mUBOMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	mGBufferShaderProgram.Bind();
	mGBufferShaderProgram.SetUniformVec3("uViewPos", glm::value_ptr(mCamera.GetWorldPosition()));
	mGBufferShaderProgram.SetUniformVec3("uMaterial.specular", glm::value_ptr(glm::vec3(0.1f)));

	mDeferredShaderProgram.Bind();
	mDeferredShaderProgram.SetUniformVec3("uViewPos", glm::value_ptr(mCamera.GetWorldPosition()));
	mDeferredShaderProgram.SetUniform1f("uMaterial.shininess", shininess);

	mDeferredShaderProgram.SetUniform1i("uNumDirLights", 1);
	mDeferredShaderProgram.SetUniformMat4("uDirLights[0].lightSpaceMat", glm::value_ptr(DIR_LIGHT_SPACE_MAT));
	mDeferredShaderProgram.SetUniformVec3("uDirLights[0].direction", glm::value_ptr(LIGHT_DIRECTION));
	mDeferredShaderProgram.SetUniformVec3("uDirLights[0].ambient", glm::value_ptr(ambientColor));
	mDeferredShaderProgram.SetUniformVec3("uDirLights[0].diffuse", glm::value_ptr(diffuseColor));
	mDeferredShaderProgram.SetUniformVec3("uDirLights[0].specular", glm::value_ptr(specularColor));

	mDeferredShaderProgram.SetUniform1i("uNumPointLights", 4);
	for (int i = 0; i < 4; i++)
	{
		glm::vec3 ambientColor = glm::vec3(0.025f) * POINT_LIGHT_COLORS[i];
		glm::vec3 diffuseColor = glm::vec3(1.0f) * POINT_LIGHT_COLORS[i];
		glm::vec3 specularColor = glm::vec3(1.0f) * POINT_LIGHT_COLORS[i];

		mDeferredShaderProgram.SetUniformVec3(std::format("uPointLights[{}].position", i), glm::value_ptr(POINT_LIGHT_POSITIONS[i]));
		mDeferredShaderProgram.SetUniformVec3(std::format("uPointLights[{}].ambient", i), glm::value_ptr(ambientColor));
		mDeferredShaderProgram.SetUniformVec3(std::format("uPointLights[{}].diffuse", i), glm::value_ptr(diffuseColor));
		mDeferredShaderProgram.SetUniformVec3(std::format("uPointLights[{}].specular", i), glm::value_ptr(specularColor));
		mDeferredShaderProgram.SetUniform1f(std::format("uPointLights[{}].constant", i), 1.0f);
		mDeferredShaderProgram.SetUniform1f(std::format("uPointLights[{}].linear", i), 0.7f);
		mDeferredShaderProgram.SetUniform1f(std::format("uPointLights[{}].quadratic", i), 1.8f);
		mDeferredShaderProgram.SetUniform1f(std::format("uPointLights[{}].farPlane", i), 100.0f);
	}

	//mSkyboxShaderProgram.Bind();
	//mSkyboxShaderProgram.SetUniformMat4("uView", glm::value_ptr(glm::mat4(glm::mat3(view))));
	//mSkyboxShaderProgram.SetUniformMat4("uProjection", glm::value_ptr(projection));
	//mCubemap.BindTexture(0);
	//mCubemap.Draw();
}

void Graphics::Engine::OnMouseMove(float xpos, float ypos)
{
	if (mIsFirstMouseMove)
	{
		mLastMouseXPos = xpos;
		mLastMouseYPos = ypos;
		mIsFirstMouseMove = false;
	}

	float xOffset = xpos - mLastMouseXPos;
	float yOffset = mLastMouseYPos - ypos;

	mLastMouseXPos = xpos;
	mLastMouseYPos = ypos;

	mCamera.Rotate(xOffset, yOffset);

}

void Graphics::Engine::OnMouseScroll(float xOffset, float yOffset)
{
	mCamera.Zoom(yOffset);
}

void Graphics::Engine::DrawModels(
	const std::vector<std::shared_ptr<Model>>& models,
	ShaderProgram& shader,
	bool outline
)
{
	if (outline)
	{
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
	}

	shader.Bind();
	for (unsigned int i = 0; i < models.size(); i++)
	{
		models[i]->Draw(shader);
	}

	if (outline)
	{
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);

		mOutlineShaderProgram.Bind();
		for (unsigned int i = 0; i < models.size(); i++)
		{
			Core::Transform transform = models[i]->GetTransform();
			transform.Scale *= 1.1f;

			models[i]->Draw(mOutlineShaderProgram, transform);
		}

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
	}
}

void Graphics::Engine::DrawTransparentModels(const std::vector<std::shared_ptr<Model>>& models, ShaderProgram& shader)
{
	shader.Bind();

	std::map<float, std::shared_ptr<Model>> sorted;
	for (unsigned int i = 0; i < models.size(); i++)
	{
		Core::Transform transform = models[i]->GetTransform();
		glm::vec3 v = mCamera.GetWorldPosition() - transform.Position;
		float distanceSquared = glm::dot(v, v);
		sorted[distanceSquared] = models[i];
	}

	for (auto it = sorted.rbegin(); it != sorted.rend(); it++)
	{
		it->second->Draw(shader);
	}
}

unsigned int Graphics::Engine::LoadTexture(const char* path, bool flip, bool srgb)
{
	unsigned int textureId = GLLoadTextureFromFile(path, flip, srgb);
	mLoadedTextures[path] = textureId;
	return textureId;
}

void Graphics::Engine::ImportModels(const std::vector<Core::ModelImport>& imports, std::vector<std::shared_ptr<Model>>* models)
{
	for (const Core::ModelImport& modelImport : imports)
	{
		std::shared_ptr<Model> model = std::make_shared<Model>(modelImport.flipTexturesVertically);

		for (const Core::TextureImport& textureImport : modelImport.textureImports)
		{
			auto it = mLoadedTextures.find(textureImport.path);

			bool srgb = textureImport.type == Core::Diffuse;

			if (it != mLoadedTextures.end())
			{
				model->SetDefaultTexture({ it->second, textureImport.type });
			}
			else
			{
				unsigned int textureId = GLLoadTextureFromFile(textureImport.path, modelImport.flipTexturesVertically, srgb);
				mLoadedTextures[textureImport.path] = textureId;
				model->SetDefaultTexture({ textureId, textureImport.type });
			}
		}

		if (!model->HasDefaultTexture(Core::Diffuse))
		{
			model->SetDefaultTexture(mDefaultTexture);
		}

		model->SetTransform(modelImport.transform);
		model->Load(modelImport.path);

		models->push_back(model);
	}
}
