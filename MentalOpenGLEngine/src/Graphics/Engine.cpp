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
	mCamera(glm::vec3(0.0f, 0.0f, 3.0f), 5.0f, 20.0f),
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

bool Graphics::Engine::Init()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, mTitle, NULL, NULL);
	if (mWindow == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		return false;
	}

	glfwMakeContextCurrent(mWindow);

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
	mLightCubeShaderProgram.Build("src/Shaders/lightCube.vert", "src/Shaders/lightCube.frag");

	BuildBuffers();

	stbi_set_flip_vertically_on_load(true);

	constexpr size_t nTextures = 3;
	BuildTextureOptions optionList[nTextures]{
		{"resources/textures/container2.png", "uMaterial.diffuse"},
		{"resources/textures/container2_specular.png", "uMaterial.specular"},
		{"resources/textures/matrix.jpg", "uMaterial.emission"}
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
	BuildTextureOptions optionList[],
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
	glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 0.0f) + glm::vec3(sin(Time::LastFrame), 0.0f, cos(Time::LastFrame)) * 3.0f;

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // set color for clearing
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // use set color to clear color buffer

	glm::mat4 projection = glm::perspective(
		glm::radians(mCamera.GetZoom()),
		static_cast<float>(mWindowWidth) / static_cast<float>(mWindowHeight),
		0.1f,
		100.0f
	);
	glm::mat4 view = mCamera.GetViewMatrix();
	glm::mat4 model(1.0f);

	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
	glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
	glm::vec3 specularColor = lightColor * glm::vec3(1.0f);

	mBaseShaderProgram.Bind();

	mBaseShaderProgram.SetUniformMat4("uView", glm::value_ptr(view));
	mBaseShaderProgram.SetUniformMat4("uProjection", glm::value_ptr(projection));
	mBaseShaderProgram.SetUniformMat4("uModel", glm::value_ptr(model));

	mBaseShaderProgram.SetUniformVec3("uViewPos", glm::value_ptr(mCamera.GetWorldPosition()));

	mBaseShaderProgram.SetUniform1f("uMaterial.shininess", 128.0f);

	mBaseShaderProgram.SetUniformVec3("uLight.ambient", glm::value_ptr(ambientColor));
	mBaseShaderProgram.SetUniformVec3("uLight.diffuse", glm::value_ptr(diffuseColor));
	mBaseShaderProgram.SetUniformVec3("uLight.specular", glm::value_ptr(specularColor));
	mBaseShaderProgram.SetUniformVec3("uLight.position", glm::value_ptr(lightPos));

	float emissionShift = Time::LastFrame * 0.5f;
	emissionShift -= (int)emissionShift;
	std::cout << emissionShift << std::endl;
	mBaseShaderProgram.SetUniform1f("emissionShift", emissionShift);

	for (size_t i = 0; i < mTextureIDs.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, mTextureIDs[i]);
	}

	glBindVertexArray(mCubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	model = glm::mat4(1.0f);
	model = glm::translate(model, lightPos);
	model = glm::scale(model, glm::vec3(0.2f));

	mLightCubeShaderProgram.Bind();
	mLightCubeShaderProgram.SetUniformMat4("uView", glm::value_ptr(view));
	mLightCubeShaderProgram.SetUniformMat4("uProjection", glm::value_ptr(projection));
	mLightCubeShaderProgram.SetUniformMat4("uModel", glm::value_ptr(model));
	mLightCubeShaderProgram.SetUniformVec3("uLightColor", glm::value_ptr(lightColor));

	glBindVertexArray(mLightVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);

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
