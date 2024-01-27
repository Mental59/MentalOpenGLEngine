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

Graphics::Engine* Graphics::Engine::mInstance(nullptr);

static float gMixAlpha = 0.2f;
static constexpr int CUBE_COUNT = 10;
static glm::vec3 CUBE_POSITIONS[CUBE_COUNT] = {
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

Graphics::Engine::Engine(const int windowWidth, const int windowHeight, const char* title) :
	mWindowWidth(windowWidth),
	mWindowHeight(windowHeight),
	mTitle(title),
	mWindow(nullptr),
	mShaderProgram(),
	mVBO(0), mVAO(0), mEBO(0)
{
	mInstance = this;
}

Graphics::Engine::~Engine()
{
	glDeleteVertexArrays(1, &mVAO);

	glDeleteBuffers(1, &mVBO);
	glDeleteBuffers(1, &mEBO);

	glDeleteTextures(mTextureIDs.size(), mTextureIDs.data());

	glfwTerminate();
}

void OnResizeCallback(GLFWwindow* window, int width, int height)
{
	Graphics::Engine::GetInstance()->OnResize(window, width, height);
}

bool Graphics::Engine::Init()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, "OpenGL Engine", NULL, NULL);
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

	glfwSetFramebufferSizeCallback(mWindow, OnResizeCallback);

	mShaderProgram.Build("src/Shaders/vertexShader.vert", "src/Shaders/fragmentShader.frag");
	BuildBuffers();

	stbi_set_flip_vertically_on_load(true);

	constexpr size_t nTextures = 2;
	BuildTextureOptions optionList[nTextures]{
		{"resources/textures/container.jpg", GL_RGB, GL_RGB},
		{"resources/textures/awesomeface.png", GL_RGBA, GL_RGBA }
	};
	BuildTextures(optionList, nTextures);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe mode
	glEnable(GL_DEPTH_TEST);

	return true;
}

void Graphics::Engine::BuildBuffers()
{
	GLfloat cubeVertices[] = {
		// positions          // texture coordinates
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mEBO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	// VAO stores calls to glVertexAttribPointer, glEnableVertexAttribArray, glDisableVertexAttribArray, glBindBuffer for GL_ELEMENT_ARRAY_BUFFER
	glBindVertexArray(mVAO);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectangleIndices), rectangleIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)0); // VBO must be bound before calling this function
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0); // unbind VAO

	glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind VBO

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind EBO (do it after unbinding VAO otherwise VAO will store unbind call)
}

void Graphics::Engine::BuildTextures(BuildTextureOptions optionList[], size_t n)
{
	mTextureIDs.reserve(n);

	mShaderProgram.Bind();

	for (size_t i = 0; i < n; i++)
	{
		GLuint textureID;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int width, height, numChannels;
		unsigned char* data = stbi_load(optionList[i].path, &width, &height, &numChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, optionList[i].internalFormat, width, height, 0, optionList[i].format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << std::format("Failed to load the texture {}", optionList[i].path) << std::endl;
		}

		mShaderProgram.SetUniform1i(std::format("uTexture{}", i + 1), i);

		mTextureIDs.push_back(textureID);

		stbi_image_free(data);
	}

	mShaderProgram.Unbind();
}

void Graphics::Engine::Run()
{
	while (!glfwWindowShouldClose(mWindow))
	{
		OnInput();

		OnRender();

		glfwPollEvents();
	}
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

	if (glfwGetKey(mWindow, GLFW_KEY_UP) == GLFW_PRESS)
	{
		gMixAlpha = gMixAlpha > 1.0f ? 1.0f : gMixAlpha + 0.001f;
	}

	if (glfwGetKey(mWindow, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		gMixAlpha = gMixAlpha < 0.0f ? 0.0f : gMixAlpha - 0.001f;
	}
}

void Graphics::Engine::OnRender()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // set color for clearing
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // use set color to clear color buffer

	float time = static_cast<float>(glfwGetTime());

	glm::mat4 view(1.0f);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

	glm::mat4 projection = glm::perspective(
		glm::radians(45.0f),
		//static_cast<float>(mWindowWidth) / static_cast<float>(mWindowHeight),
		16.0f / 9.0f,
		0.1f,
		100.0f
	);

	mShaderProgram.Bind();

	for (size_t i = 0; i < mTextureIDs.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, mTextureIDs[i]);
	}

	mShaderProgram.SetUniform1f("uMixAlpha", gMixAlpha);

	mShaderProgram.SetUniformMatrix4fv("uView", glm::value_ptr(view));
	mShaderProgram.SetUniformMatrix4fv("uProjection", glm::value_ptr(projection));

	glBindVertexArray(mVAO);

	for (int i = 0; i < CUBE_COUNT; i++)
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, CUBE_POSITIONS[i]);

		float angle = 20.0f * i;
		if (i % 3 == 0)
		{
			angle = time * 30.0f;
		}
		model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

		mShaderProgram.SetUniformMatrix4fv("uModel", glm::value_ptr(model));

		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	glfwSwapBuffers(mWindow);
}
