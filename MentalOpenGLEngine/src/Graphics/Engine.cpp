#include "Engine.h"

#include <iostream>
#include <format>
#include <glad/glad.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "External/stb_image.h"
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
	mLightCubeShaderProgram(),
	mVBO(0), mCubeVAO(0), mEBO(0), mLightVAO(0),
	mCamera(glm::vec3(0.0f, 0.0f, 3.0f), 5.0f, 0.1f),
	mLastMouseXPos(0.0f), mLastMouseYPos(0.0f), mIsFirstMouseMove(true)
{
	mInstance = this;
}

Graphics::Engine::~Engine()
{
	glDeleteVertexArrays(1, &mCubeVAO);
	glDeleteVertexArrays(1, &mLightVAO);

	glDeleteBuffers(1, &mVBO);
	glDeleteBuffers(1, &mEBO);

	if (mTextureIDs.size() > 0)
	{
		glDeleteTextures(mTextureIDs.size(), mTextureIDs.data());
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

	mBaseShaderProgram.Build("src/Shaders/base.vert", "src/Shaders/multipleLights.frag");
	mLightCubeShaderProgram.Build("src/Shaders/lightCube.vert", "src/Shaders/lightCube.frag");

	BuildBuffers();

	stbi_set_flip_vertically_on_load(true);

	constexpr size_t nTextures = 2;
	Core::BuildTextureOptions optionList[nTextures]{
		{"resources/textures/container2.png", "uMaterial.diffuse"},
		{"resources/textures/container2_specular.png", "uMaterial.specular"},
	};
	BuildTextures(&mBaseShaderProgram, optionList, nTextures);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe mode
	glEnable(GL_DEPTH_TEST);

	return true;
}

void Graphics::Engine::BuildBuffers()
{
	GLfloat cubeVertices[] = {
		// back side
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,

		//front side
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,

		// left side
		-0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,

		// right side
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,

		 // bottom side
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,

		// upper side
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f
	};

	glGenVertexArrays(1, &mCubeVAO);
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mEBO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	// VAO stores calls to glVertexAttribPointer, glEnableVertexAttribArray, glDisableVertexAttribArray, glBindBuffer for GL_ELEMENT_ARRAY_BUFFER
	glBindVertexArray(mCubeVAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void*)0); // VBO must be bound before calling this function
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);


	glGenVertexArrays(1, &mLightVAO);
	glBindVertexArray(mLightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void*)0);
	glEnableVertexAttribArray(0);


	glBindVertexArray(0); // unbind VAO

	glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind VBO

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind EBO (do it after unbinding VAO otherwise VAO will store unbind call)
}

void Graphics::Engine::BuildTextures(
	ShaderProgram* shaderProgram,
	Core::BuildTextureOptions optionList[],
	size_t n
)
{
	mTextureIDs.reserve(n);

	shaderProgram->Bind();

	for (size_t i = 0; i < n; i++)
	{
		GLuint textureID;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // trilinear filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int width, height, numChannels;
		unsigned char* data = stbi_load(optionList[i].path, &width, &height, &numChannels, 0);

		GLenum format = 0;
		if (numChannels == 1)
		{
			format = GL_RED;
		}
		else if (numChannels == 3)
		{
			format = GL_RGB;
		}
		else if (numChannels == 4)
		{
			format = GL_RGBA;
		}

		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << std::format("Failed to load the texture {}", optionList[i].path) << std::endl;
		}

		shaderProgram->SetUniform1i(optionList[i].uniformName, i);

		mTextureIDs.push_back(textureID);

		stbi_image_free(data);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	shaderProgram->Unbind();
}

void Graphics::Engine::Run()
{
	while (!glfwWindowShouldClose(mWindow))
	{
		UpdateTimer();

		OnInput();

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
	static constexpr size_t cubeCount = 10;
	static constexpr size_t pointLightsCount = 4;
	static constexpr glm::vec3 cubePositions[cubeCount]{
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(2.0f, 5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f, 3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f, 2.0f, -2.5f),
		glm::vec3(1.5f, 0.2f, -1.5f),
		glm::vec3(-1.3f, 1.0f, -1.5f)
	};
	static constexpr glm::vec3 pointLightPositions[pointLightsCount]{
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};
	static constexpr glm::vec3 pointLightColors[pointLightsCount]{
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(1.0f, 0.0f, 1.0f)
	};

	glClearColor(0.05f, 0.05f, 0.05f, 1.0f); // set color for clearing
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // use set color to clear color buffer

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

	mBaseShaderProgram.SetUniformVec3("uDirectionalLight.direction", -0.2f, -1.0f, -0.3f);
	mBaseShaderProgram.SetUniformVec3("uDirectionalLight.ambient", 0.05f, 0.05f, 0.05f);
	mBaseShaderProgram.SetUniformVec3("uDirectionalLight.diffuse", 0.4f, 0.4f, 0.4f);
	mBaseShaderProgram.SetUniformVec3("uDirectionalLight.specular", 0.5f, 0.5f, 0.5f);

	mBaseShaderProgram.SetUniformVec3("uSpotLight.position", glm::value_ptr(mCamera.GetWorldPosition()));
	mBaseShaderProgram.SetUniformVec3("uSpotLight.direction", glm::value_ptr(mCamera.GetForwardDirection()));
	mBaseShaderProgram.SetUniform1f("uSpotLight.cutOffCosine", glm::cos(glm::radians(12.5f)));
	mBaseShaderProgram.SetUniform1f("uSpotLight.outerCutOffCosine", glm::cos(glm::radians(17.5f)));
	mBaseShaderProgram.SetUniformVec3("uSpotLight.ambient", 0.0f, 0.0f, 0.0f);
	mBaseShaderProgram.SetUniformVec3("uSpotLight.diffuse", 1.0f, 1.0f, 1.0f);
	mBaseShaderProgram.SetUniformVec3("uSpotLight.specular", 1.0f, 1.0f, 1.0f);
	mBaseShaderProgram.SetUniform1f("uSpotLight.constant", 1.0f);
	mBaseShaderProgram.SetUniform1f("uSpotLight.linear", 0.09f);
	mBaseShaderProgram.SetUniform1f("uSpotLight.quadratic", 0.032f);

	mBaseShaderProgram.SetUniform1i("uNumPointLights", pointLightsCount);
	for (size_t i = 0; i < pointLightsCount; i++)
	{
		mBaseShaderProgram.SetUniformVec3(std::format("uPointLights[{}].position", i), glm::value_ptr(pointLightPositions[i]));

		mBaseShaderProgram.SetUniformVec3(std::format("uPointLights[{}].ambient", i), glm::value_ptr(pointLightColors[i] * 0.05f));
		mBaseShaderProgram.SetUniformVec3(std::format("uPointLights[{}].diffuse", i), glm::value_ptr(pointLightColors[i] * 0.4f));
		mBaseShaderProgram.SetUniformVec3(std::format("uPointLights[{}].specular", i), glm::value_ptr(pointLightColors[i] * 0.5f));

		mBaseShaderProgram.SetUniform1f(std::format("uPointLights[{}].constant", i), 1.0f);
		mBaseShaderProgram.SetUniform1f(std::format("uPointLights[{}].linear", i), 0.07f);
		mBaseShaderProgram.SetUniform1f(std::format("uPointLights[{}].quadratic", i), 0.017f);
	}

	for (size_t i = 0; i < mTextureIDs.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, mTextureIDs[i]);
	}

	glBindVertexArray(mCubeVAO);
	for (size_t i = 0; i < cubeCount; i++)
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, cubePositions[i]);
		float angle = 20.0f * i;
		model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
		mBaseShaderProgram.SetUniformMat4("uModel", glm::value_ptr(model));

		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	mLightCubeShaderProgram.Bind();
	mLightCubeShaderProgram.SetUniformMat4("uView", glm::value_ptr(view));
	mLightCubeShaderProgram.SetUniformMat4("uProjection", glm::value_ptr(projection));

	glBindVertexArray(mLightVAO);
	for (size_t i = 0; i < pointLightsCount; i++)
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, pointLightPositions[i]);
		model = glm::scale(model, glm::vec3(0.2f));

		mLightCubeShaderProgram.SetUniformMat4("uModel", glm::value_ptr(model));
		mLightCubeShaderProgram.SetUniformVec3("uLightColor", glm::value_ptr(pointLightColors[i]));

		glDrawArrays(GL_TRIANGLES, 0, 36);
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
