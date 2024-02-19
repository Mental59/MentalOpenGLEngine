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
	{"resources/objects/sponza/sponza.obj", Core::Transform{glm::vec3(0.0f, -20.0f, 0.0f), glm::vec3(0.01f)}},
	{"resources/objects/plane/plane.obj", Core::Transform{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(15.0f)}, {{"resources/textures/metal.png", Core::Diffuse}}},

	{"resources/objects/cube/cube.obj", Core::Transform{glm::vec3(5.0f, 2.0f, -12.0f), glm::vec3(1.0f)}, {{"resources/textures/marble.jpg", Core::Diffuse}}},
	{"resources/objects/cube/cube.obj", Core::Transform{glm::vec3(10.0f, 2.0f, -12.0f), glm::vec3(1.0f)}, {{"resources/textures/marble.jpg", Core::Diffuse}}},

	{"resources/objects/cube/cube.obj", Core::Transform{glm::vec3(5.0f, 2.0f, 4.0f), glm::vec3(1.0f)}, {{"resources/textures/marble.jpg", Core::Diffuse}}},
	{"resources/objects/cube/cube.obj", Core::Transform{glm::vec3(10.0f, 2.0f, 4.0f), glm::vec3(1.0f)}, {{"resources/textures/marble.jpg", Core::Diffuse}}},
};
std::vector<Core::ModelImport> MODEL_IMPORT_CUBES{
	{"resources/objects/cube/cube.obj", Core::Transform{glm::vec3(5.0f, 5.0f, 4.0f), glm::vec3(1.0f)}, {{"resources/textures/marble.jpg", Core::Diffuse}}},
	{"resources/objects/cube/cube.obj", Core::Transform{glm::vec3(10.0f, 5.0f, 4.0f), glm::vec3(1.0f)}, {{"resources/textures/marble.jpg", Core::Diffuse}}},
};
std::vector<Core::ModelImport> MODEL_IMPORT_TRANSPARENT{
	{"resources/objects/plane/plane.obj", Core::Transform{glm::vec3(5.0f, 2.0f, -8.0f), glm::vec3(2.0f), 90.0f}, {{"resources/textures/window.png", Core::Diffuse}}},
	{"resources/objects/plane/plane.obj", Core::Transform{glm::vec3(10.0f, 2.0f, -8.0f), glm::vec3(2.0f), 90.0f}, {{"resources/textures/window.png", Core::Diffuse}}},

	{"resources/objects/plane/plane.obj", Core::Transform{glm::vec3(5.0f, 2.0f, -4.0f), glm::vec3(2.0f), 90.0f}, {{"resources/textures/window.png", Core::Diffuse}}},
	{"resources/objects/plane/plane.obj", Core::Transform{glm::vec3(10.0f, 2.0f, -4.0f), glm::vec3(2.0f), 90.0f}, {{"resources/textures/window.png", Core::Diffuse}}},

	{"resources/objects/plane/plane.obj", Core::Transform{glm::vec3(5.0f, 2.0f, 0.0f), glm::vec3(2.0f), 90.0f}, {{"resources/textures/window.png", Core::Diffuse}}},
	{"resources/objects/plane/plane.obj", Core::Transform{glm::vec3(10.0f, 2.0f, 0.0f), glm::vec3(2.0f), 90.0f}, {{"resources/textures/window.png", Core::Diffuse}}},
};

std::vector<std::shared_ptr<Model>> MODELS;
std::vector<std::shared_ptr<Model>> CUBES;
std::vector<std::shared_ptr<Model>> TRANSPARENT;


Graphics::Engine::Engine(const int windowWidth, const int windowHeight, const char* title) :
	mWindowWidth(windowWidth),
	mWindowHeight(windowHeight),
	mTitle(title),
	mWindow(nullptr),
	mBaseShaderProgram(),
	mCamera(glm::vec3(0.0f, 0.0f, 0.0f), 5.0f, 0.1f),
	mLastMouseXPos(0.0f), mLastMouseYPos(0.0f), mIsFirstMouseMove(true),
	mDefaultTexture{}
{
	mInstance = this;
}

Graphics::Engine::~Engine()
{
	for (const auto& loadedTexture : mLoadedTextures)
	{
		glDeleteTextures(1, &loadedTexture.second);
	}

	glfwTerminate();
}

void OnResizeCallback(GLFWwindow* window, int width, int height)
{
	Graphics::Engine::GetInstance()->OnResize(window, width, height);
}

void OnCursorPoseCallback(GLFWwindow* window, double xpos, double ypos)
{
	Graphics::Engine::GetInstance()->OnMouseMove(static_cast<float>(xpos), static_cast<float>(ypos));
}

void OnMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	Graphics::Engine::GetInstance()->OnMouseScroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

bool Graphics::Engine::Init(bool vsync, bool windowedFullscreen)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


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

	const GLubyte* glVersion = glGetString(GL_VERSION);
	std::cout << "OpenGL version: " << glVersion << std::endl;

	glfwSetFramebufferSizeCallback(mWindow, OnResizeCallback);
	glfwSetCursorPosCallback(mWindow, OnCursorPoseCallback);
	glfwSetScrollCallback(mWindow, OnMouseScrollCallback);

	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	mBaseShaderProgram.Build("src/Shaders/base.vert", "src/Shaders/base.frag");
	mOutlineShaderProgram.Build("src/Shaders/base.vert", "src/Shaders/outline.frag");

	mFramebufferScreenProgram.Build("src/Shaders/framebufferScreen.vert", "src/Shaders/framebufferScreen.frag");
	mFramebufferScreenProgram.Bind();
	mFramebufferScreenProgram.SetUniform1i("uScreenTexture", 0);
	mFramebufferScreenProgram.Unbind();

	// Load default diffuse texture
	unsigned int defaultDiffuseTextureId = GLLoadTextureFromFile("resources/textures/default.png");
	mLoadedTextures["resources/textures/default.png"] = defaultDiffuseTextureId;
	mDefaultTexture = { defaultDiffuseTextureId, Core::Diffuse };

	// Setup models
	ImportModels(MODEL_IMPORTS, &MODELS);
	ImportModels(MODEL_IMPORT_CUBES, &CUBES);
	ImportModels(MODEL_IMPORT_TRANSPARENT, &TRANSPARENT);

	mFrameBuffer.Create(mWindowWidth, mWindowHeight);
	mScreenQuad.Create();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe mode

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

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
	mFrameBuffer.Bind();

	glStencilMask(0xFF);
	glClearColor(0.3, 0.3, 0.3, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0x00);

	glEnable(GL_DEPTH_TEST);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	DrawScene();

	mFrameBuffer.Unbind();
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	mFramebufferScreenProgram.Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mFrameBuffer.GetTextureColorId());
	glDisable(GL_DEPTH_TEST);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	mScreenQuad.Draw();

	glfwSwapBuffers(mWindow);
}

void Graphics::Engine::DrawScene()
{
	glm::mat4 projection = glm::perspective(
		glm::radians(mCamera.GetZoom()),
		static_cast<float>(mWindowWidth) / static_cast<float>(mWindowHeight),
		0.1f,
		100.0f
	);
	glm::mat4 view = mCamera.GetViewMatrix();

	mBaseShaderProgram.Bind();

	mBaseShaderProgram.SetUniformMat4("uView", glm::value_ptr(view));
	mBaseShaderProgram.SetUniformMat4("uProjection", glm::value_ptr(projection));
	mBaseShaderProgram.SetUniformVec3("uViewPos", glm::value_ptr(mCamera.GetWorldPosition()));
	mBaseShaderProgram.SetUniform1f("uMaterial.shininess", 128.0f);

	mBaseShaderProgram.SetUniformVec3("uLight.direction", -0.2f, -1.0f, -0.3f);
	mBaseShaderProgram.SetUniformVec3("uLight.ambient", 0.2f, 0.2f, 0.2f);
	mBaseShaderProgram.SetUniformVec3("uLight.diffuse", 0.5f, 0.5f, 0.5f);
	mBaseShaderProgram.SetUniformVec3("uLight.specular", 1.0f, 1.0f, 1.0f);

	mOutlineShaderProgram.Bind();

	mOutlineShaderProgram.SetUniformMat4("uView", glm::value_ptr(view));
	mOutlineShaderProgram.SetUniformMat4("uProjection", glm::value_ptr(projection));
	mOutlineShaderProgram.SetUniformVec3("uOutlineColor", 1, 0, 1);

	DrawModels(MODELS, mBaseShaderProgram, false);
	DrawModels(CUBES, mBaseShaderProgram, false);
	DrawTransparentModels(TRANSPARENT, mBaseShaderProgram);
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
		std::shared_ptr<Model> model = std::make_shared<Model>();

		for (const Core::TextureImport& textureImport : modelImport.textureImports)
		{
			auto it = mLoadedTextures.find(textureImport.path);
			if (it != mLoadedTextures.end())
			{
				model->SetDefaultTexture({ it->second, textureImport.type });
			}
			else
			{
				unsigned int textureId = GLLoadTextureFromFile(textureImport.path);
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
