#include "Engine.h"
#include <iostream>
#include <format>
#include <glad/glad.h>
#include <glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <random>
#include <map>
#include "Shader.h"
#include "Camera.h"
#include "Time.h"
#include "Utils.h"

Graphics::Engine* Graphics::Engine::mInstance(nullptr);

static float FPS_COUNT = 0.0f;
static float LAST_FPS_SHOW_SECONDS_ELAPSED = 0.0f;

static Model SPHERE_MODEL;
static Model CERBERUS_MODEL;

static Model SPHERE_MODEL_1;
static Model SPHERE_MODEL_2;
static Model SPHERE_MODEL_3;
static Model SPHERE_MODEL_4;
static Model SPHERE_MODEL_5;
static Model SPHERE_MODEL_6;
static Model SPHERE_MODEL_7;

static glm::vec3 LIGHT_DIRECTION = glm::normalize(glm::vec3(1.0f, -0.5f, 1.0f));
static glm::mat4 DIR_LIGHT_SPACE_MAT;
static glm::vec3 DIR_LIGHT_POS;

static constexpr int NUM_POINT_LIGHTS = 4;
static glm::vec3 POINT_LIGHT_POSITIONS[NUM_POINT_LIGHTS]{
	glm::vec3(-10.0f,  10.0f, 5.0f),
	glm::vec3(10.0f,  10.0f, 5.0f),
	glm::vec3(-10.0f, -10.0f, 5.0f),
	glm::vec3(10.0f, -10.0f, 5.0f),
};

static glm::vec3 POINT_LIGHT_COLORS[NUM_POINT_LIGHTS]{
	glm::vec3(1.0f, 1.0f, 1.0f) * 300.0f,
	glm::vec3(1.0f, 1.0f, 1.0f) * 300.0f,
	glm::vec3(1.0f, 1.0f, 1.0f) * 300.0f,
	glm::vec3(1.0f, 1.0f, 1.0f) * 300.0f,
};

static std::vector<glm::vec3> BACKPACK_POSITIONS{
		glm::vec3(-5.0, -11.5, -5.0),
		//glm::vec3(0.0, -0.5, -3.0),
		//glm::vec3(3.0, -0.5, -3.0),
		//glm::vec3(-6.0, -0.5, -3.0),
		//glm::vec3(6.0, -0.5, -3.0),

		//glm::vec3(-3.0, -0.5, 0.0),
		glm::vec3(0.0, -11.5, 0.0),
		//glm::vec3(3.0, -0.5, 0.0),
		//glm::vec3(-6.0, -0.5, 0.0),
		//glm::vec3(6.0, -0.5, 0.0),

		//glm::vec3(-3.0, -0.5, 3.0),
		//glm::vec3(0.0, -0.5, 3.0),
		glm::vec3(5.0, -11.5, 5.0),
		//glm::vec3(-6.0, -0.5, 3.0),
		//glm::vec3(6.0, -0.5, 3.0),

		//glm::vec3(-3.0, -0.5, -6.0),
		//glm::vec3(0.0, -0.5, -6.0),
		//glm::vec3(3.0, -0.5, -6.0),
		glm::vec3(-10.0, -11.5, -10.0),
		//glm::vec3(6.0, -0.5, -6.0),

		//glm::vec3(-3.0, -0.5, 6.0),
		//glm::vec3(0.0, -0.5, 6.0),
		//glm::vec3(3.0, -0.5, 6.0),
		//glm::vec3(-6.0, -0.5, 6.0),
		glm::vec3(10.0, -11.5, 10.0),
};

static constexpr int NUM_SSAO_KERNEL_SAMPLES = 64;
static constexpr int SSAO_NOISE_TEXTURE_SIZE = 4;
static constexpr int NUM_SSAO_NOISE_SAMPLES = SSAO_NOISE_TEXTURE_SIZE * SSAO_NOISE_TEXTURE_SIZE;
static glm::vec3 SSAO_KERNEL[NUM_SSAO_KERNEL_SAMPLES];
static glm::vec3 SSAO_NOISE[NUM_SSAO_NOISE_SAMPLES];

Graphics::Engine::Engine(const int windowWidth, const int windowHeight, const char* title) :
	mWindowWidth(windowWidth),
	mWindowHeight(windowHeight),
	mAspectRatio(static_cast<float>(windowWidth) / static_cast<float>(windowHeight)),
	mTitle(title),
	mWindow(nullptr),
	mBaseShaderProgram(),
	mCamera(glm::vec3(0.0f, 0.0f, 15.0f), 5.0f, 0.1f),
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
	glDeleteTextures(1, &mNoiseTexture);

	glfwTerminate();
}

static void OnResizeCallback(GLFWwindow* window, int width, int height)
{
	// TODO	: Needs to be fixed
	//Graphics::Engine::GetInstance()->OnResize(window, width, height);
}

static void OnCursorPoseCallback(GLFWwindow* window, double xpos, double ypos)
{
	Graphics::Engine::GetInstance()->OnMouseMove(static_cast<float>(xpos), static_cast<float>(ypos));
}

static void OnMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	Graphics::Engine::GetInstance()->OnMouseScroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

static float Lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

static void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return; // ignore these non-significant error codes

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

bool Graphics::Engine::Init(bool vsync, bool windowedFullscreen)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
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

	if (!mTextRenderer.Build(mWindowWidth, mWindowHeight))
	{
		return false;
	}

#if _DEBUG
	// enable OpenGL debug context if context allows for debug context
	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		std::cout << "OpenGL debug output enabled" << std::endl;
	}
#endif

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
	Shader dirShadowMappingVertexInstancedShader("src/Shaders/directionalShadowMappingInstanced.vert", Shader::Vertex);

	Shader pointShadowMappingVertShader("src/Shaders/pointShadowMapping.vert", Shader::Vertex);
	Shader pointShadowMappingFragShader("src/Shaders/pointShadowMapping.frag", Shader::Fragment);
	Shader pointShadowMappingGeomShader("src/Shaders/pointShadowMapping.geom", Shader::Geometry);
	Shader pointShadowMappingVertInstancedShader("src/Shaders/pointShadowMappingInstanced.vert", Shader::Vertex);

	Shader gaussianBlurVertShader("src/Shaders/gaussianBlur.vert", Shader::Vertex);
	Shader gaussianBlurFragShader("src/Shaders/gaussianBlur.frag", Shader::Fragment);

	Shader gBufferVertShader("src/Shaders/gBuffer.vert", Shader::Vertex);
	Shader gBufferFragShader("src/Shaders/gBuffer.frag", Shader::Fragment);
	Shader gBufferInstancedVertShader("src/Shaders/gBufferInstanced.vert", Shader::Vertex);

	Shader deferredVertShader("src/Shaders/deferred.vert", Shader::Vertex);
	Shader deferredFragShader("src/Shaders/deferred.frag", Shader::Fragment);
	Shader deferredPBRFragShader("src/Shaders/deferredPBR.frag", Shader::Fragment);

	Shader ssaoVertShader("src/Shaders/ssao.vert", Shader::Vertex);
	Shader ssaoFragShader("src/Shaders/ssao.frag", Shader::Fragment);
	Shader ssaoBlurFragShader("src/Shaders/ssaoBlur.frag", Shader::Fragment);

	mBaseShaderProgram.Build({ baseVertexShader, baseFragmentShader });
	//mBaseInstancedShaderProgram.Build({ baseInstancedVertexShader, baseFragmentShader });
	mOutlineShaderProgram.Build({ baseVertexShader, outlineFragmentShader });
	mEnvironmentMappingShaderProgram.Build({ baseVertexShader, environmentMappingFragmentShader });
	mSkyboxShaderProgram.Build({ cubemapVertexShader, cubemapFragmentShader });
	mPostProcessingShaderProgram.Build({ framebufferVertexShader, framebufferFragmentShader });
	mNormalsVisualizationShaderProgram.Build({ normalsVisualizationVertexShader, normalsVisualizationFragmentShader, normalsVisualizationGeometryShader });
	mLightSourceShaderProgram.Build({ lightSourceFragmentShader, lightSourceVertexShader });
	mDirectionalShadowMappingShaderProgram.Build({ dirShadowMappingFragmentShader, dirShadowMappingVertexShader });
	mPointShadowMappingShaderProgram.Build({ pointShadowMappingVertShader, pointShadowMappingFragShader, pointShadowMappingGeomShader });
	mGaussianBlurShaderProgram.Build({ gaussianBlurVertShader, gaussianBlurFragShader });
	mGBufferShaderProgram.Build({ gBufferVertShader, gBufferFragShader });
	mDeferredShaderProgram.Build({ deferredVertShader, deferredPBRFragShader });
	mGBufferInstancedShaderProgram.Build({ gBufferInstancedVertShader, gBufferFragShader });
	mDirectionalShadowMappingInstancedShaderProgram.Build({ dirShadowMappingFragmentShader, dirShadowMappingVertexInstancedShader });
	mPointShadowMappingInstancedShaderProgram.Build({ pointShadowMappingVertInstancedShader, pointShadowMappingFragShader, pointShadowMappingGeomShader });
	mSSAOShaderProgram.Build({ ssaoVertShader, ssaoFragShader });
	mSSAOBlurShaderProgram.Build({ ssaoVertShader, ssaoBlurFragShader });

	// Setting texture units
	mPostProcessingShaderProgram.Bind();
	mPostProcessingShaderProgram.SetUniform1i("uScreenTexture", 0);
	mPostProcessingShaderProgram.SetUniform1i("uBloomTexture", 1);
	mPostProcessingShaderProgram.Unbind();
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
	mDeferredShaderProgram.SetUniform1i("uPointLights[0].shadowCubeMap", 3);
	mDeferredShaderProgram.SetUniform1i("uSSAOTexture", 4);
	mDeferredShaderProgram.SetUniform1i("uIrradianceMap", 5);
	mDeferredShaderProgram.SetUniform1i("gMetallicRoughnessAO", 6);
	mDeferredShaderProgram.SetUniform1i("uPrefilterMap", 7);
	mDeferredShaderProgram.SetUniform1i("uBrdfLutMap", 8);
	mDeferredShaderProgram.Unbind();
	mSSAOShaderProgram.Bind();
	mSSAOShaderProgram.SetUniform1i("gPosition", 0);
	mSSAOShaderProgram.SetUniform1i("gNormal", 1);
	mSSAOShaderProgram.SetUniform1i("uNoiseTexture", 2);
	mSSAOShaderProgram.Unbind();
	mSSAOBlurShaderProgram.Bind();
	mSSAOBlurShaderProgram.SetUniform1i("uSSAOTexture", 0);
	mSSAOBlurShaderProgram.Unbind();

	//Setting uniform block bindings
	unsigned int uniformMatricesBlockBinding = 0;
	mBaseShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);
	//mBaseInstancedShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);
	mOutlineShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);
	mEnvironmentMappingShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);
	mLightSourceShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);
	mGBufferShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);
	mGBufferInstancedShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);
	mSSAOShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);

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
	unsigned int defaultAlbedoTextureId = GLLoadTextureFromFile("resources/textures/default.png", false, true);
	mLoadedTextures["resources/textures/default.png"] = defaultAlbedoTextureId;
	mDefaultTexture = { defaultAlbedoTextureId, Core::Albedo };

	mDeferredLightingFrameBuffer.Create(mWindowWidth, mWindowHeight);
	mGFrameBuffer.Create(mWindowWidth, mWindowHeight);
	mSSAOFrameBuffer.Create(mWindowWidth, mWindowHeight);
	mSSAOBlurFrameBuffer.Create(mWindowWidth, mWindowHeight);
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

	SPHERE_MODEL.SetDefaultTexture({ LoadTexture("resources/textures/rusted_iron/albedo.png"), Core::Albedo });
	SPHERE_MODEL.SetDefaultTexture({ LoadTexture("resources/textures/rusted_iron/metallic.png"), Core::Metallic });
	SPHERE_MODEL.SetDefaultTexture({ LoadTexture("resources/textures/rusted_iron/roughness.png"), Core::Roughness });
	SPHERE_MODEL.SetDefaultTexture({ LoadTexture("resources/textures/rusted_iron/ao.png"), Core::AmbientOcclusion });
	SPHERE_MODEL.SetDefaultTexture({ LoadTexture("resources/textures/rusted_iron/normal.png"), Core::Normal });
	SPHERE_MODEL.Load("resources/objects/sphere/sphere.obj");

	CERBERUS_MODEL.SetDefaultTexture({ LoadTexture("resources/objects/cerberus/textures/Cerberus_A.tga"), Core::Albedo });
	CERBERUS_MODEL.SetDefaultTexture({ LoadTexture("resources/objects/cerberus/textures/Cerberus_M.tga"), Core::Metallic });
	CERBERUS_MODEL.SetDefaultTexture({ LoadTexture("resources/objects/cerberus/textures/Cerberus_N.tga"), Core::Normal });
	CERBERUS_MODEL.SetDefaultTexture({ LoadTexture("resources/objects/cerberus/textures/Cerberus_R.tga"), Core::Roughness });
	CERBERUS_MODEL.SetDefaultTexture({ LoadTexture("resources/objects/cerberus/textures/Raw/Cerberus_AO.tga"), Core::AmbientOcclusion });
	CERBERUS_MODEL.Load("resources/objects/cerberus/cerberus.FBX");

	SPHERE_MODEL_1.SetDefaultTexture({ LoadTexture("resources/textures/pbr/cloudy-veined-quartz-bl/cloudy-veined-quartz_albedo.png"), Core::Albedo });
	SPHERE_MODEL_1.SetDefaultTexture({ LoadTexture("resources/textures/pbr/cloudy-veined-quartz-bl/cloudy-veined-quartz_ao.png"), Core::AmbientOcclusion });
	SPHERE_MODEL_1.SetDefaultTexture({ LoadTexture("resources/textures/pbr/cloudy-veined-quartz-bl/cloudy-veined-quartz_metallic.png"), Core::Metallic });
	SPHERE_MODEL_1.SetDefaultTexture({ LoadTexture("resources/textures/pbr/cloudy-veined-quartz-bl/cloudy-veined-quartz_normal-ogl.png"), Core::Normal });
	SPHERE_MODEL_1.SetDefaultTexture({ LoadTexture("resources/textures/pbr/cloudy-veined-quartz-bl/cloudy-veined-quartz_roughness.png"), Core::Roughness });
	SPHERE_MODEL_1.Load("resources/objects/sphere/sphere.obj");

	SPHERE_MODEL_2.SetDefaultTexture({ LoadTexture("resources/textures/pbr/cloudy-veined-quartz-light-bl/cloudy-veined-quartz-light_albedo.png"), Core::Albedo });
	SPHERE_MODEL_2.SetDefaultTexture({ LoadTexture("resources/textures/pbr/cloudy-veined-quartz-light-bl/cloudy-veined-quartz-light_ao.png"), Core::AmbientOcclusion });
	SPHERE_MODEL_2.SetDefaultTexture({ LoadTexture("resources/textures/pbr/cloudy-veined-quartz-light-bl/cloudy-veined-quartz-light_metallic.png"), Core::Metallic });
	SPHERE_MODEL_2.SetDefaultTexture({ LoadTexture("resources/textures/pbr/cloudy-veined-quartz-light-bl/cloudy-veined-quartz-light_normal-ogl.png"), Core::Normal });
	SPHERE_MODEL_2.SetDefaultTexture({ LoadTexture("resources/textures/pbr/cloudy-veined-quartz-light-bl/cloudy-veined-quartz-light_roughness.png"), Core::Roughness });
	SPHERE_MODEL_2.Load("resources/objects/sphere/sphere.obj");

	SPHERE_MODEL_3.SetDefaultTexture({ LoadTexture("resources/textures/pbr/columned-lava-rock-bl/columned-lava-rock_albedo.png"), Core::Albedo });
	SPHERE_MODEL_3.SetDefaultTexture({ LoadTexture("resources/textures/pbr/columned-lava-rock-bl/columned-lava-rock_ao.png"), Core::AmbientOcclusion });
	SPHERE_MODEL_3.SetDefaultTexture({ LoadTexture("resources/textures/pbr/columned-lava-rock-bl/columned-lava-rock_height.png"), Core::Height });
	SPHERE_MODEL_3.SetDefaultTexture({ LoadTexture("resources/textures/pbr/columned-lava-rock-bl/columned-lava-rock_metallic.png"), Core::Metallic });
	SPHERE_MODEL_3.SetDefaultTexture({ LoadTexture("resources/textures/pbr/columned-lava-rock-bl/columned-lava-rock_normal-ogl.png"), Core::Normal });
	SPHERE_MODEL_3.SetDefaultTexture({ LoadTexture("resources/textures/pbr/columned-lava-rock-bl/columned-lava-rock_roughness.png"), Core::Roughness });
	SPHERE_MODEL_3.Load("resources/objects/sphere/sphere.obj");

	SPHERE_MODEL_4.SetDefaultTexture({ LoadTexture("resources/textures/pbr/gold-nugget-bl/gold-nugget1_albedo.png"), Core::Albedo });
	SPHERE_MODEL_4.SetDefaultTexture({ LoadTexture("resources/textures/pbr/gold-nugget-bl/gold-nugget1_ao.png"), Core::AmbientOcclusion });
	SPHERE_MODEL_4.SetDefaultTexture({ LoadTexture("resources/textures/pbr/gold-nugget-bl/gold-nugget1_height.png"), Core::Height });
	SPHERE_MODEL_4.SetDefaultTexture({ LoadTexture("resources/textures/pbr/gold-nugget-bl/gold-nugget1_metallic.png"), Core::Metallic });
	SPHERE_MODEL_4.SetDefaultTexture({ LoadTexture("resources/textures/pbr/gold-nugget-bl/gold-nugget1_normal-ogl.png"), Core::Normal });
	SPHERE_MODEL_4.SetDefaultTexture({ LoadTexture("resources/textures/pbr/gold-nugget-bl/gold-nugget1_roughness.png"), Core::Roughness });
	SPHERE_MODEL_4.Load("resources/objects/sphere/sphere.obj");

	SPHERE_MODEL_5.SetDefaultTexture({ LoadTexture("resources/textures/pbr/ice-field-bl/ice_field_albedo.png"), Core::Albedo });
	SPHERE_MODEL_5.SetDefaultTexture({ LoadTexture("resources/textures/pbr/ice-field-bl/ice_field_ao.png"), Core::AmbientOcclusion });
	SPHERE_MODEL_5.SetDefaultTexture({ LoadTexture("resources/textures/pbr/ice-field-bl/ice_field_height.png"), Core::Height });
	SPHERE_MODEL_5.SetDefaultTexture({ LoadTexture("resources/textures/pbr/ice-field-bl/ice_field_metallic.png"), Core::Metallic });
	SPHERE_MODEL_5.SetDefaultTexture({ LoadTexture("resources/textures/pbr/ice-field-bl/ice_field_normal-ogl.png"), Core::Normal });
	SPHERE_MODEL_5.SetDefaultTexture({ LoadTexture("resources/textures/pbr/ice-field-bl/ice_field_roughness.png"), Core::Roughness });
	SPHERE_MODEL_5.Load("resources/objects/sphere/sphere.obj");

	SPHERE_MODEL_6.SetDefaultTexture({ LoadTexture("resources/textures/pbr/patchy-meadow1-bl/patchy-meadow1_albedo.png"), Core::Albedo });
	SPHERE_MODEL_6.SetDefaultTexture({ LoadTexture("resources/textures/pbr/patchy-meadow1-bl/patchy-meadow1_ao.png"), Core::AmbientOcclusion });
	SPHERE_MODEL_6.SetDefaultTexture({ LoadTexture("resources/textures/pbr/patchy-meadow1-bl/patchy-meadow1_height.png"), Core::Height });
	SPHERE_MODEL_6.SetDefaultTexture({ LoadTexture("resources/textures/pbr/patchy-meadow1-bl/patchy-meadow1_metallic.png"), Core::Metallic });
	SPHERE_MODEL_6.SetDefaultTexture({ LoadTexture("resources/textures/pbr/patchy-meadow1-bl/patchy-meadow1_normal-ogl.png"), Core::Normal });
	SPHERE_MODEL_6.SetDefaultTexture({ LoadTexture("resources/textures/pbr/patchy-meadow1-bl/patchy-meadow1_roughness.png"), Core::Roughness });
	SPHERE_MODEL_6.Load("resources/objects/sphere/sphere.obj");

	SPHERE_MODEL_7.SetDefaultTexture({ LoadTexture("resources/textures/pbr/white-quilted-diamond-bl/white-quilted-diamond_albedo.png"), Core::Albedo });
	SPHERE_MODEL_7.SetDefaultTexture({ LoadTexture("resources/textures/pbr/white-quilted-diamond-bl/white-quilted-diamond_ao.png"), Core::AmbientOcclusion });
	SPHERE_MODEL_7.SetDefaultTexture({ LoadTexture("resources/textures/pbr/white-quilted-diamond-bl/white-quilted-diamond_height.png"), Core::Height });
	SPHERE_MODEL_7.SetDefaultTexture({ LoadTexture("resources/textures/pbr/white-quilted-diamond-bl/white-quilted-diamond_metallic.png"), Core::Metallic });
	SPHERE_MODEL_7.SetDefaultTexture({ LoadTexture("resources/textures/pbr/white-quilted-diamond-bl/white-quilted-diamond_normal-ogl.png"), Core::Normal });
	SPHERE_MODEL_7.SetDefaultTexture({ LoadTexture("resources/textures/pbr/white-quilted-diamond-bl/white-quilted-diamond_roughness.png"), Core::Roughness });
	SPHERE_MODEL_7.Load("resources/objects/sphere/sphere.obj");

	//ssao kernel
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine generator;
	for (int i = 0; i < NUM_SSAO_KERNEL_SAMPLES; i++)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator)
		);
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);

		// scale samples s.t. they're more aligned to center of kernel
		float scale = static_cast<float>(i) / static_cast<float>(NUM_SSAO_KERNEL_SAMPLES);
		scale = Lerp(0.1f, 1.0f, scale * scale);

		sample *= scale;

		SSAO_KERNEL[i] = sample;
	}
	//ssao noise
	for (int i = 0; i < NUM_SSAO_NOISE_SAMPLES; i++)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f
		);

		SSAO_NOISE[i] = noise;
	}

	glGenTextures(1, &mNoiseTexture);
	glBindTexture(GL_TEXTURE_2D, mNoiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SSAO_NOISE_TEXTURE_SIZE, SSAO_NOISE_TEXTURE_SIZE, 0, GL_RGB, GL_FLOAT, SSAO_NOISE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe mode

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);
	//glEnable(GL_BLEND);
	//glDisable(GL_BLEND);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	//glEnable(GL_MULTISAMPLE);
	//glEnable(GL_FRAMEBUFFER_SRGB);
	glDisable(GL_MULTISAMPLE);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	mHDRMap.Setup("resources/hdr/little_paris_eiffel_tower_4k.hdr", 4096, 64);
	mCubemap.Load(mHDRMap.GetCubeMapTextureId());

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
	//static float yaw = 0.0f;
	//mCamera.SetYaw(yaw);
	//yaw += -Time::DeltaTime * 30.0f;

	//for (int i = 0; i < NUM_POINT_LIGHTS; i++)
	//{
	//	POINT_LIGHT_POSITIONS[i].x = sin(Time::LastFrame) * 4.0f;
	//	POINT_LIGHT_POSITIONS[i].z = cos(Time::LastFrame) * 4.0f;
	//}
}

void Graphics::Engine::UpdateTimer()
{
	float currentFrame = static_cast<float>(glfwGetTime());
	Time::DeltaTime = currentFrame - Time::LastFrame;
	Time::LastFrame = currentFrame;

	LAST_FPS_SHOW_SECONDS_ELAPSED += Time::DeltaTime;
	if (LAST_FPS_SHOW_SECONDS_ELAPSED >= 0.25f)
	{
		FPS_COUNT = 1.0f / Time::DeltaTime;
		LAST_FPS_SHOW_SECONDS_ELAPSED = 0.0f;
	}
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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//Render to depth map
	//ShadowPass();

	glViewport(0, 0, mWindowWidth, mWindowHeight);

	// Geometry pass
	mGFrameBuffer.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 viewMatrix = mCamera.GetViewMatrix();
	glm::mat4 projectionMatrix = mCamera.GetProjectionMatrix(mAspectRatio);
	SetupScene(viewMatrix, projectionMatrix);
	DrawScene(mGBufferShaderProgram, &mGBufferInstancedShaderProgram);

	// SSAO
	mSSAOFrameBuffer.Bind();
	glClear(GL_COLOR_BUFFER_BIT);
	mSSAOShaderProgram.Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mGFrameBuffer.GetPositionTextureId());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mGFrameBuffer.GetNormalTextureId());
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mNoiseTexture);

	glm::vec2 noiseScale(
		static_cast<float>(mWindowWidth) / static_cast<float>(SSAO_NOISE_TEXTURE_SIZE),
		static_cast<float>(mWindowHeight) / static_cast<float>(SSAO_NOISE_TEXTURE_SIZE)
	);
	mSSAOShaderProgram.SetUniformVec2("uNoiseScale", glm::value_ptr(noiseScale));
	mSSAOShaderProgram.SetUniform1i("uNumSamples", NUM_SSAO_KERNEL_SAMPLES);
	mSSAOShaderProgram.SetUniform1f("uPower", 5.0f);
	for (int i = 0; i < NUM_SSAO_KERNEL_SAMPLES; i++)
	{
		mSSAOShaderProgram.SetUniformVec3(std::format("uSamples[{}]", i), glm::value_ptr(SSAO_KERNEL[i]));
	}

	glDisable(GL_DEPTH_TEST);
	mScreenQuad.Draw();
	glEnable(GL_DEPTH_TEST);

	mSSAOBlurFrameBuffer.Bind();
	glClear(GL_COLOR_BUFFER_BIT);
	mSSAOBlurShaderProgram.Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mSSAOFrameBuffer.GetTextureColorId());

	glDisable(GL_DEPTH_TEST);
	mScreenQuad.Draw();
	glEnable(GL_DEPTH_TEST);

	// Lighting pass
	mDeferredLightingFrameBuffer.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mDeferredShaderProgram.Bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mGFrameBuffer.GetPositionTextureId());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mGFrameBuffer.GetNormalTextureId());
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mGFrameBuffer.GetAlbedoSpecularTextureId());
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mPointDepthMap.GetTextureColorId());
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, mSSAOBlurFrameBuffer.GetTextureColorId());
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mHDRMap.GetIrradianceMapTextureId());
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, mGFrameBuffer.GetMetallicRoughnessAOTextureId());
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mHDRMap.GetPrefilterMapTextureId());
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, mHDRMap.GetBRDFLutTextureId());

	glDisable(GL_DEPTH_TEST);
	mScreenQuad.Draw();
	glEnable(GL_DEPTH_TEST);

	// Forward pass
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mGFrameBuffer.GetFrameBufferId());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDeferredLightingFrameBuffer.GetFrameBufferId());
	glBlitFramebuffer(0, 0, mWindowWidth, mWindowHeight, 0, 0, mWindowWidth, mWindowHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	mDeferredLightingFrameBuffer.Bind();
	mLightSourceShaderProgram.Bind();
	for (int i = 0; i < NUM_POINT_LIGHTS; i++)
	{
		mLightSourceShaderProgram.SetUniformVec3("uLightColor", glm::value_ptr(POINT_LIGHT_COLORS[i]));
		glm::mat4 lightSourceMat = glm::mat4(1.0f);
		lightSourceMat = glm::translate(lightSourceMat, POINT_LIGHT_POSITIONS[i]);
		lightSourceMat = glm::scale(lightSourceMat, glm::vec3(0.15f));
		SPHERE_MODEL.Draw(mLightSourceShaderProgram, lightSourceMat);
	}

	// Drawing skybox
	mSkyboxShaderProgram.Bind();
	mSkyboxShaderProgram.SetUniformMat4("uView", glm::value_ptr(glm::mat4(glm::mat3(viewMatrix))));
	mSkyboxShaderProgram.SetUniformMat4("uProjection", glm::value_ptr(projectionMatrix));
	//mSkyboxShaderProgram.SetUniform1f("uGamma", 2.2f);
	//mSkyboxShaderProgram.SetUniform1f("uExposure", 1.0f);
	mCubemap.BindTexture(0);
	mCubemap.Draw();
	mCubemap.UnbindTexture();

	// Blurring bright fragments with two-pass Gaussian Blur
	//mGaussianBlurShaderProgram.Bind();
	//mGaussianBlurShaderProgram.SetUniformVec2("uSampleDistance", glm::value_ptr(glm::vec2(1.0f, 1.0f)));
	//bool isHorizontal = true, isFirstIteration = true;
	//unsigned int numPasses = 5;
	//glDisable(GL_DEPTH_TEST);
	//for (unsigned int i = 0; i < numPasses * 2; i++)
	//{
	//	mGaussianBlurShaderProgram.SetUniform1i("uHorizontal", isHorizontal);
	//	glBindFramebuffer(GL_FRAMEBUFFER, mPingPongFrameBuffers[isHorizontal]);
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, isFirstIteration ? mDeferredLightingFrameBuffer.GetBrightTextureColorId() : mPingPongColorBuffers[!isHorizontal]);
	//	mScreenQuad.Draw();
	//	isHorizontal = !isHorizontal;
	//	isFirstIteration = false;
	//}
	//glEnable(GL_DEPTH_TEST);

	// Post-processing
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mPostProcessingShaderProgram.Bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mDeferredLightingFrameBuffer.GetTextureColorId());
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, mPingPongColorBuffers[!isHorizontal]);

	glDisable(GL_DEPTH_TEST);
	mScreenQuad.Draw();
	glEnable(GL_BLEND);
	mTextRenderer.Draw("(C) LearnOpenGL.com", glm::vec2(25.0f, 25.0f), glm::vec3(0.5, 0.8f, 0.2f), 0.5f);
	mTextRenderer.Draw(
		std::format("FPS: {}", static_cast<int>(FPS_COUNT)),
		glm::vec2(20.0f, static_cast<float>(mWindowHeight) - 30.0f),
		glm::vec3(0.5, 0.8f, 0.2f),
		0.5f
	);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

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
	mDirectionalShadowMappingInstancedShaderProgram.Bind();
	mDirectionalShadowMappingInstancedShaderProgram.SetUniformMat4("uLightSpaceMatrix", glm::value_ptr(DIR_LIGHT_SPACE_MAT));

	mPointShadowMappingShaderProgram.Bind();
	for (unsigned int i = 0; i < 6; i++)
	{
		mPointShadowMappingShaderProgram.SetUniformMat4(
			std::format("uShadowMatrices[{}]", i), glm::value_ptr(pointShadowTransforms[i])
		);
	}
	mPointShadowMappingShaderProgram.SetUniformVec3("uLightPos", glm::value_ptr(POINT_LIGHT_POSITIONS[0]));
	mPointShadowMappingShaderProgram.SetUniform1f("uFarPlane", farPlane);

	mPointShadowMappingInstancedShaderProgram.Bind();
	for (unsigned int i = 0; i < 6; i++)
	{
		mPointShadowMappingInstancedShaderProgram.SetUniformMat4(
			std::format("uShadowMatrices[{}]", i), glm::value_ptr(pointShadowTransforms[i])
		);
	}
	mPointShadowMappingInstancedShaderProgram.SetUniformVec3("uLightPos", glm::value_ptr(POINT_LIGHT_POSITIONS[0]));
	mPointShadowMappingInstancedShaderProgram.SetUniform1f("uFarPlane", farPlane);

	//glViewport(0, 0, mDirectionalDepthMap.GetWidth(), mDirectionalDepthMap.GetHeight());
	//mDirectionalDepthMap.Bind();
	//glClear(GL_DEPTH_BUFFER_BIT);
	//DrawScene(mDirectionalShadowMappingShaderProgram, &mDirectionalShadowMappingInstancedShaderProgram);

	glViewport(0, 0, mPointDepthMap.GetWidth(), mPointDepthMap.GetHeight());
	mPointDepthMap.Bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	DrawScene(mPointShadowMappingShaderProgram, &mPointShadowMappingInstancedShaderProgram);
}

void Graphics::Engine::DrawScene(
	ShaderProgram& shader,
	ShaderProgram* const shaderInstanced
)
{
	static const int nRows = 7;
	static const int nColumns = 7;

	shader.Bind();

	glm::mat4 model;

	//model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0));
	//model = glm::scale(model, glm::vec3(12.5f, 12.5f, 12.5f));
	//shader.SetUniform1f("uNormalsMultiplier", -1.0f);
	//shader.SetUniform1f("uTexTiling", 4.0f);
	//glDisable(GL_CULL_FACE);
	//FLOOR_MODEL.Draw(shader, model);
	//glEnable(GL_CULL_FACE);
	//shader.SetUniform1f("uTexTiling", 1.0f);
	//shader.SetUniform1f("uNormalsMultiplier", 1.0f);

	//constexpr float spacing = 2.5;
	//for (int i = 0; i < nRows; i++)
	//{
	//	shader.SetUniform1f("uMaterial.metallic", (float)i / (float)nRows); // increases from bottom to the top
	//	for (int j = 0; j < nColumns; j++)
	//	{
	//		shader.SetUniform1f("uMaterial.roughness", glm::clamp((float)j / (float)nColumns, 0.05f, 1.0f)); // increases from left to right
	//		shader.SetUniform1f("uMaterial.roughness", glm::clamp((float)j / (float)nColumns, 0.05f, 1.0f)); // increases from left to right

	//		model = glm::mat4(1.0f);
	//		glm::vec3 pos(
	//			(j - (nColumns / 2)) * spacing,
	//			(i - (nRows / 2)) * spacing,
	//			0.0f
	//		);
	//		model = glm::translate(model, pos);
	//		SPHERE_MODEL.Draw(shader, model);
	//	}
	//}

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(20.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.25f));
	CERBERUS_MODEL.Draw(shader, model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	SPHERE_MODEL_1.Draw(shader, model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-2.5f, 0.0f, 0.0f));
	SPHERE_MODEL_2.Draw(shader, model);

	//model = glm::mat4(1.0f);
	//model = glm::translate(model, glm::vec3(-5.0f, 0.0f, 0.0f));
	//SPHERE_MODEL_3.Draw(shader, model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-5.0f, 0.0f, 0.0f));
	SPHERE_MODEL_4.Draw(shader, model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-7.5f, 0.0f, 0.0f));
	SPHERE_MODEL_5.Draw(shader, model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-10.0f, 0.0f, 0.0f));
	SPHERE_MODEL_6.Draw(shader, model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-12.5f, 0.0f, 0.0f));
	SPHERE_MODEL_7.Draw(shader, model);

	//if (shaderInstanced)
	//{
	//	shaderInstanced->Bind();
	//	BACKPACK_MODEL.DrawInstanced(*shaderInstanced, BACKPACK_POSITIONS.size());
	//}
	//else
	//{
	//	for (int i = 0; i < BACKPACK_POSITIONS.size(); i++)
	//	{
	//		glm::mat4 model = glm::translate(glm::mat4(1.0f), BACKPACK_POSITIONS[i]);
	//		model = glm::scale(model, glm::vec3(0.5f));
	//		BACKPACK_MODEL.Draw(shader, model);
	//	}
	//}
}

void Graphics::Engine::SetupScene(
	const glm::mat4& view,
	const glm::mat4& projection
)
{
	glBindBuffer(GL_UNIFORM_BUFFER, mUBOMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	mGBufferShaderProgram.Bind();
	mGBufferShaderProgram.SetUniformVec3("uViewPos", glm::value_ptr(mCamera.GetWorldPosition()));
	mGBufferShaderProgram.SetUniform1f("uMaterial.ambientOcclusion", 1.0f);
	mGBufferShaderProgram.SetUniformVec3("uMaterial.albedo", 0.5f, 0.0f, 0.0f);
	mGBufferShaderProgram.SetUniform1i("useTextures", true);

	mDeferredShaderProgram.Bind();
	mDeferredShaderProgram.SetUniformVec3("uViewPos", glm::value_ptr(mCamera.GetWorldPosition()));

	mDeferredShaderProgram.SetUniform1i("uNumPointLights", NUM_POINT_LIGHTS);
	for (int i = 0; i < NUM_POINT_LIGHTS; i++)
	{
		mDeferredShaderProgram.SetUniformVec3(std::format("uPointLights[{}].position", i), glm::value_ptr(POINT_LIGHT_POSITIONS[i]));
		mDeferredShaderProgram.SetUniformVec3(std::format("uPointLights[{}].color", i), glm::value_ptr(POINT_LIGHT_COLORS[i]));
		mDeferredShaderProgram.SetUniform1f(std::format("uPointLights[{}].farPlane", i), 100.0f);
	}
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

			bool srgb = textureImport.type == Core::Albedo;

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

		if (!model->HasDefaultTexture(Core::Albedo))
		{
			model->SetDefaultTexture(mDefaultTexture);
		}

		model->SetTransform(modelImport.transform);
		model->Load(modelImport.path);

		models->push_back(model);
	}
}
