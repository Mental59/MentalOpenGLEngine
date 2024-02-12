#include "Engine.h"

#include <iostream>
#include <format>
#include <glad/glad.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Time.h"

Graphics::Engine* Graphics::Engine::mInstance(nullptr);

Graphics::Engine::Engine(const int windowWidth, const int windowHeight, const char* title) :
	mWindowWidth(windowWidth),
	mWindowHeight(windowHeight),
	mTitle(title),
	mWindow(nullptr),
	mBaseShaderProgram(),
	mCamera(glm::vec3(0.0f, 0.0f, 3.0f), 5.0f, 0.1f),
	mLastMouseXPos(0.0f), mLastMouseYPos(0.0f), mIsFirstMouseMove(true),
	mModelImports{
		{ "resources/objects/sponza/sponza.obj", Core::Transform{glm::vec3(0.0f), glm::vec3(0.01f)}},
		{ "resources/objects/cube/cube.obj", Core::Transform{glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(1.0f)}},
		{ "resources/objects/plane/plane.obj", Core::Transform{glm::vec3(0.0f, 7.0f, 0.0f), glm::vec3(1.0f)} },
	}
{
	mInstance = this;
}

Graphics::Engine::~Engine()
{
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
	mNoTextureShaderProgram.Build("src/Shaders/base.vert", "src/Shaders/noTextures.frag");

	for (const auto& modelImport : mModelImports)
	{
		std::shared_ptr<Model> model = std::make_shared<Model>();
		model->Load(modelImport.path);
		mModels.push_back(model);
	}

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe mode
	glEnable(GL_DEPTH_TEST);

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
	glClearColor(0.4, 0.48, 0.48, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

	mNoTextureShaderProgram.Bind();

	mNoTextureShaderProgram.SetUniformMat4("uView", glm::value_ptr(view));
	mNoTextureShaderProgram.SetUniformMat4("uProjection", glm::value_ptr(projection));
	mNoTextureShaderProgram.SetUniformVec3("uViewPos", glm::value_ptr(mCamera.GetWorldPosition()));
	mNoTextureShaderProgram.SetUniform1f("uMaterial.shininess", 128.0f);

	mNoTextureShaderProgram.SetUniformVec3("uLight.direction", -0.2f, -1.0f, -0.3f);
	mNoTextureShaderProgram.SetUniformVec3("uLight.ambient", 0.2f, 0.2f, 0.2f);
	mNoTextureShaderProgram.SetUniformVec3("uLight.diffuse", 0.5f, 0.5f, 0.5f);
	mNoTextureShaderProgram.SetUniformVec3("uLight.specular", 1.0f, 1.0f, 1.0f);

	for (unsigned int i = 0; i < mModels.size(); i++)
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, mModelImports[i].transform.Position);
		model = glm::scale(model, mModelImports[i].transform.Scale);

		if (mModels[i]->HasTextures())
		{
			mBaseShaderProgram.Bind();
			mBaseShaderProgram.SetUniformMat4("uModel", glm::value_ptr(model));
			mModels[i]->Draw(mBaseShaderProgram);
		}
		else
		{
			mNoTextureShaderProgram.Bind();
			mNoTextureShaderProgram.SetUniformMat4("uModel", glm::value_ptr(model));
			mModels[i]->Draw(mNoTextureShaderProgram);
		}
	}

	glfwSwapBuffers(mWindow);
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
