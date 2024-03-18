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

std::vector<Core::ModelImport> MODEL_IMPORTS{
	//{"resources/objects/sponza/sponza.obj", Core::Transform{glm::vec3(0.0f, -20.0f, 0.0f), glm::vec3(0.01f)}},
	{"resources/objects/plane/plane.obj", Core::Transform{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(15.0f, 1.0f, 15.0f)}, false, {{"resources/textures/wood.png", Core::Diffuse}}},

	{"resources/objects/cube/cube.obj", Core::Transform{glm::vec3(4.0f, 4.0f, 1.0f), glm::vec3(1.0f), 45.0f}, true, {{"resources/textures/container2.png", Core::Diffuse}, {"resources/textures/container2_specular.png", Core::Specular}}},
	{"resources/objects/cube/cube.obj", Core::Transform{glm::vec3(8.0f, 1.0f, 1.0f), glm::vec3(1.0f)}, true, {{"resources/textures/container2.png", Core::Diffuse}, {"resources/textures/container2_specular.png", Core::Specular}}},

	{"resources/objects/cube/cube.obj", Core::Transform{glm::vec3(5.5f, 3.0f, 4.0f), glm::vec3(1.0f), 0.0f}, false, {{"resources/textures/container2.png", Core::Diffuse}, {"resources/textures/container2_specular.png", Core::Specular}}},
	{"resources/objects/cube/cube.obj", Core::Transform{glm::vec3(8.0f, 1.0f, 4.0f), glm::vec3(1.0f), 0.0f}, false, {{"resources/textures/container2.png", Core::Diffuse}, {"resources/textures/container2_specular.png", Core::Specular}}},
};
std::vector<Core::ModelImport> MODEL_IMPORT_SPHERES{
	{"resources/objects/sphere/sphere.obj", Core::Transform{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f)}},
	{"resources/objects/sphere/sphere.obj", Core::Transform{glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(1.0f)}},
	{"resources/objects/sphere/sphere.obj", Core::Transform{glm::vec3(6.0f, 0.0f, 0.0f), glm::vec3(1.0f)}},
	{"resources/objects/sphere/sphere.obj", Core::Transform{glm::vec3(-3.0f, 0.0f, 0.0f), glm::vec3(1.0f)}},
	{"resources/objects/sphere/sphere.obj", Core::Transform{glm::vec3(-6.0f, 0.0f, 0.0f), glm::vec3(1.0f)}},

	{"resources/objects/sphere/sphere.obj", Core::Transform{glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(1.0f)}},
	{"resources/objects/sphere/sphere.obj", Core::Transform{glm::vec3(3.0f, 3.0f, 0.0f), glm::vec3(1.0f)}},
	{"resources/objects/sphere/sphere.obj", Core::Transform{glm::vec3(6.0f, 3.0f, 0.0f), glm::vec3(1.0f)}},
	{"resources/objects/sphere/sphere.obj", Core::Transform{glm::vec3(-3.0f, 3.0f, 0.0f), glm::vec3(1.0f)}},
	{"resources/objects/sphere/sphere.obj", Core::Transform{glm::vec3(-6.0f, 3.0f, 0.0f), glm::vec3(1.0f)}},

	{"resources/objects/sphere/sphere.obj", Core::Transform{glm::vec3(0.0f, 6.0f, 0.0f), glm::vec3(1.0f)}},
	{"resources/objects/sphere/sphere.obj", Core::Transform{glm::vec3(3.0f, 6.0f, 0.0f), glm::vec3(1.0f)}},
	{"resources/objects/sphere/sphere.obj", Core::Transform{glm::vec3(6.0f, 6.0f, 0.0f), glm::vec3(1.0f)}},
	{"resources/objects/sphere/sphere.obj", Core::Transform{glm::vec3(-3.0f, 6.0f, 0.0f), glm::vec3(1.0f)}},
	{"resources/objects/sphere/sphere.obj", Core::Transform{glm::vec3(-6.0f, 6.0f, 0.0f), glm::vec3(1.0f)}},

	{"resources/objects/backpack/backpack.obj", Core::Transform{glm::vec3(0.0f, 12.0f, 0.0f), glm::vec3(1.0f)}, true}
};
std::vector<Core::ModelImport> MODEL_IMPORT_TRANSPARENT{
	{"resources/objects/plane/plane.obj", Core::Transform{glm::vec3(5.0f, 2.0f, -8.0f), glm::vec3(2.0f), 90.0f}, false, {{"resources/textures/window.png", Core::Diffuse}}},
	{"resources/objects/plane/plane.obj", Core::Transform{glm::vec3(10.0f, 2.0f, -8.0f), glm::vec3(2.0f), 90.0f}, false, {{"resources/textures/window.png", Core::Diffuse}}},

	{"resources/objects/plane/plane.obj", Core::Transform{glm::vec3(5.0f, 2.0f, -4.0f), glm::vec3(2.0f), 90.0f}, false, {{"resources/textures/window.png", Core::Diffuse}}},
	{"resources/objects/plane/plane.obj", Core::Transform{glm::vec3(10.0f, 2.0f, -4.0f), glm::vec3(2.0f), 90.0f}, false, {{"resources/textures/window.png", Core::Diffuse}}},

	{"resources/objects/plane/plane.obj", Core::Transform{glm::vec3(5.0f, 2.0f, 0.0f), glm::vec3(2.0f), 90.0f}, false, {{"resources/textures/window.png", Core::Diffuse}}},
	{"resources/objects/plane/plane.obj", Core::Transform{glm::vec3(10.0f, 2.0f, 0.0f), glm::vec3(2.0f), 90.0f}, false, {{"resources/textures/window.png", Core::Diffuse}}},
};

std::vector<std::shared_ptr<Model>> MODELS;
std::vector<std::shared_ptr<Model>> SPHERES;
std::vector<std::shared_ptr<Model>> TRANSPARENT;

Model ASTEROID_MODEL;
Model MARS_MODEL;
Model SPHERE_MODEL;

glm::vec3 LIGHT_DIRECTION = glm::vec3(2.0f, -4.0f, 1.0f);
glm::mat4 LIGHT_SPACE_MAT;

//constexpr int ASTEROIDS_NUM = 100000;
//glm::mat4 ASTEROID_TRANSFORMS[ASTEROIDS_NUM];

Graphics::Engine::Engine(const int windowWidth, const int windowHeight, const char* title) :
	mWindowWidth(windowWidth),
	mWindowHeight(windowHeight),
	mAspectRatio(static_cast<float>(windowWidth) / static_cast<float>(windowHeight)),
	mTitle(title),
	mWindow(nullptr),
	mBaseShaderProgram(),
	mCamera(glm::vec3(0.0f, 0.0f, 3.0f), 5.0f, 0.1f),
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
	Shader shadowMappingVertexShader("src/Shaders/shadowMapping.vert", Shader::Vertex);
	Shader shadowMappingFragmentShader("src/Shaders/shadowMapping.frag", Shader::Fragment);

	mBaseShaderProgram.Build({ baseVertexShader, baseFragmentShader });
	mBaseInstancedShaderProgram.Build({ baseInstancedVertexShader, baseFragmentShader });
	mOutlineShaderProgram.Build({ baseVertexShader, outlineFragmentShader });
	mEnvironmentMappingShaderProgram.Build({ baseVertexShader, environmentMappingFragmentShader });
	mSkyboxShaderProgram.Build({ cubemapVertexShader, cubemapFragmentShader });
	mFramebufferScreenShaderProgram.Build({ framebufferVertexShader, framebufferFragmentShader });
	mNormalsVisualizationShaderProgram.Build({ normalsVisualizationVertexShader, normalsVisualizationFragmentShader, normalsVisualizationGeometryShader });
	mLightSourceShaderProgram.Build({ lightSourceFragmentShader, lightSourceVertexShader });
	mShadowMappingShaderProgram.Build({ shadowMappingFragmentShader, shadowMappingVertexShader });

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
	mBaseShaderProgram.Unbind();

	//Setting uniform block bindings
	unsigned int uniformMatricesBlockBinding = 0;
	mBaseShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);
	mBaseInstancedShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);
	mOutlineShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);
	mEnvironmentMappingShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);
	mLightSourceShaderProgram.SetUniformBlockBinding("Matrices", uniformMatricesBlockBinding);

	size_t bufferSize = 2 * sizeof(glm::mat4);
	glGenBuffers(1, &mUBOMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER, mUBOMatrices);
	glBufferData(GL_UNIFORM_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, uniformMatricesBlockBinding, mUBOMatrices, 0, bufferSize);

	mDepthMap.Build(2048, 2048);

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

	// Setup models
	ImportModels(MODEL_IMPORTS, &MODELS);
	//ImportModels(MODEL_IMPORT_SPHERES, &SPHERES);
	//ImportModels(MODEL_IMPORT_TRANSPARENT, &TRANSPARENT);

	int samples = 4;
	mFrameBuffer.Create(mWindowWidth, mWindowHeight, samples);
	//mRearViewFrameBuffer.Create(mWindowWidth, mWindowHeight);
	mScreenQuad.Create();

	//unsigned int rockTextureId = GLLoadTextureFromFile("resources/objects/rock/rock.png");
	//mLoadedTextures["resources/objects/rock/rock.png"] = rockTextureId;
	//ASTEROID_MODEL.SetDefaultTexture({ rockTextureId, Core::Diffuse });
	//ASTEROID_MODEL.Load("resources/objects/rock/rock.obj");
	//MARS_MODEL.Load("resources/objects/planet/planet.obj");
	SPHERE_MODEL.Load("resources/objects/sphere/sphere.obj");

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

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glEnable(GL_MULTISAMPLE);

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
		OnRender();
		glfwPollEvents();
	}
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
	glViewport(0, 0, mDepthMap.GetWidth(), mDepthMap.GetHeight());
	mDepthMap.Bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	//glCullFace(GL_FRONT);
	ShadowPass();
	//glCullFace(GL_BACK);

	//Drawing scene
	glViewport(0, 0, mWindowWidth, mWindowHeight);
	mFrameBuffer.Bind();
	glStencilMask(0xFF);
	glClearColor(0.3, 0.3, 0.3, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0x00);
	DrawScene(mCamera.GetViewMatrix(), mCamera.GetProjectionMatrix(mAspectRatio));
	mFrameBuffer.Unbind();

	//Post processing
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	mFramebufferScreenShaderProgram.Bind();
	mFramebufferScreenShaderProgram.SetUniformMat4("uModelMat", glm::value_ptr(glm::mat4(1.0f)));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mFrameBuffer.GetTextureColorId());
	glDisable(GL_DEPTH_TEST);
	mScreenQuad.Draw();
	glEnable(GL_DEPTH_TEST);

	glfwSwapBuffers(mWindow);
}

void Graphics::Engine::ShadowPass()
{
	//setting up matrices
	float nearPlane = 0.1f, farPlane = 100.0f;
	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, nearPlane, farPlane);
	glm::mat4 lightView = glm::lookAt(
		-LIGHT_DIRECTION * 3.0f,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	LIGHT_SPACE_MAT = lightProjection * lightView;

	mShadowMappingShaderProgram.Bind();
	mShadowMappingShaderProgram.SetUniformMat4("uLightSpaceMatrix", glm::value_ptr(LIGHT_SPACE_MAT));

	DrawModels(MODELS, mShadowMappingShaderProgram, false);
}

void Graphics::Engine::DrawScene(
	const glm::mat4& view,
	const glm::mat4& projection
)
{
	//static glm::vec3 lightColor(0.0f, 0.5f, 0.7f);
	static glm::vec3 lightColor(1.f, 1.f, 1.f);
	static glm::vec3 ambientColor = glm::vec3(0.03f, 0.03f, 0.03f) * lightColor;
	static glm::vec3 diffuseColor = glm::vec3(0.4f, 0.4f, 0.4f) * lightColor;
	static glm::vec3 specularColor = glm::vec3(1.0f, 1.0f, 1.0f) * lightColor;

	static glm::vec3 pointLightPos(0.0f, 2.0f, 0.0f);
	static float shininess = 32.0f;

	glBindBuffer(GL_UNIFORM_BUFFER, mUBOMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	mBaseShaderProgram.Bind();
	mBaseShaderProgram.SetUniformVec3("uViewPos", glm::value_ptr(mCamera.GetWorldPosition()));
	mBaseShaderProgram.SetUniform1f("uMaterial.shininess", shininess);
	mBaseShaderProgram.SetUniformVec3("uDirLight.direction", glm::value_ptr(LIGHT_DIRECTION));
	mBaseShaderProgram.SetUniformVec3("uDirLight.ambient", glm::value_ptr(ambientColor));
	mBaseShaderProgram.SetUniformVec3("uDirLight.diffuse", glm::value_ptr(diffuseColor));
	mBaseShaderProgram.SetUniformVec3("uDirLight.specular", glm::value_ptr(specularColor));
	//mBaseShaderProgram.SetUniform1f("uTexTiling", 1.0f);
	//mBaseShaderProgram.SetUniformVec2("uTexDisplacement", glm::value_ptr(glm::vec2(0.0f, 0.0f)));
	mBaseShaderProgram.SetUniform1i("uNumPointLights", 0);
	mBaseShaderProgram.SetUniformVec3("uPointLights[0].position", glm::value_ptr(pointLightPos));
	mBaseShaderProgram.SetUniformVec3("uPointLights[0].ambient", glm::value_ptr(ambientColor));
	mBaseShaderProgram.SetUniformVec3("uPointLights[0].diffuse", glm::value_ptr(diffuseColor));
	mBaseShaderProgram.SetUniformVec3("uPointLights[0].specular", glm::value_ptr(specularColor));
	mBaseShaderProgram.SetUniformVec3("uPointLights[0].specular", glm::value_ptr(specularColor));
	mBaseShaderProgram.SetUniform1f("uPointLights[0].constant", 1.0f);
	mBaseShaderProgram.SetUniform1f("uPointLights[0].linear", 0.022f);
	mBaseShaderProgram.SetUniform1f("uPointLights[0].quadratic", 0.0019f);
	mBaseShaderProgram.SetUniformMat4("uLightSpaceMatrix", glm::value_ptr(LIGHT_SPACE_MAT));
	glActiveTexture(GL_TEXTURE16);
	glBindTexture(GL_TEXTURE_2D, mDepthMap.GetTextureColorId());

	mBaseInstancedShaderProgram.Bind();
	mBaseInstancedShaderProgram.SetUniformVec3("uViewPos", glm::value_ptr(mCamera.GetWorldPosition()));
	mBaseInstancedShaderProgram.SetUniform1f("uMaterial.shininess", shininess);
	mBaseInstancedShaderProgram.SetUniformVec3("uDirLight.direction", glm::value_ptr(LIGHT_DIRECTION));
	mBaseInstancedShaderProgram.SetUniformVec3("uDirLight.ambient", glm::value_ptr(ambientColor));
	mBaseInstancedShaderProgram.SetUniformVec3("uDirLight.diffuse", glm::value_ptr(diffuseColor));
	mBaseInstancedShaderProgram.SetUniformVec3("uDirLight.specular", glm::value_ptr(specularColor));

	mOutlineShaderProgram.Bind();
	mOutlineShaderProgram.SetUniformVec3("uOutlineColor", 1, 0, 1);

	mNormalsVisualizationShaderProgram.Bind();
	mNormalsVisualizationShaderProgram.SetUniformMat4("uView", glm::value_ptr(view));
	mNormalsVisualizationShaderProgram.SetUniformMat4("uProjection", glm::value_ptr(projection));

	//mEnvironmentMappingShaderProgram.Bind();
	//mEnvironmentMappingShaderProgram.SetUniformVec3("uViewPos", glm::value_ptr(mCamera.GetWorldPosition()));
	//mCubemap.BindTexture(0);

	//mBaseShaderProgram.Bind();
	//glm::mat4 marsModelMat(1.0f);
	//marsModelMat = glm::translate(marsModelMat, pointLightPos);
	//marsModelMat = glm::rotate(marsModelMat, glm::radians((float)glfwGetTime()) * 5.0f, glm::vec3(0.0f, 0.5f, 1.0f));
	//marsModelMat = glm::scale(marsModelMat, glm::vec3(0.2f));
	//MARS_MODEL.Draw(mBaseShaderProgram, marsModelMat);

	//mBaseInstancedShaderProgram.Bind();
	//ASTEROID_MODEL.DrawInstanced(mBaseInstancedShaderProgram, ASTEROIDS_NUM);

	DrawModels(MODELS, mBaseShaderProgram, false);
	//DrawModels(MODELS, mNormalsVisualizationShaderProgram, false);

	//mLightSourceShaderProgram.Bind();
	//glm::mat4 lightSourceMat(1.0f);
	//lightSourceMat = glm::translate(lightSourceMat, pointLightPos);
	//lightSourceMat = glm::scale(lightSourceMat, glm::vec3(0.1f));
	//SPHERE_MODEL.Draw(mLightSourceShaderProgram, lightSourceMat);

	//DrawModels(SPHERES, mBaseShaderProgram, false);
	//DrawModels(SPHERES, mNormalsVisualizationShaderProgram, false);

	//DrawTransparentModels(TRANSPARENT, mBaseShaderProgram);

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
