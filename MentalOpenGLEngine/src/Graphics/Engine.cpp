#include "Engine.h"

#include <iostream>
#include <glad/glad.h>
#include <glfw3.h>
#include "Shader.h"

Graphics::Engine* Graphics::Engine::mInstance(nullptr);

Graphics::Engine::Engine(const int windowWidth, const int windowHeight, const char* title) :
	mWindowWidth(windowWidth),
	mWindowHeight(windowHeight),
	mTitle(title),
	mWindow(nullptr)
{
	mInstance = this;
}

Graphics::Engine::~Engine()
{
	glDeleteProgram(mShaderProgram);

	glDeleteVertexArrays(1, &mVAO);

	glDeleteBuffers(1, &mVBO);
	glDeleteBuffers(1, &mEBO);

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

	glfwSetFramebufferSizeCallback(mWindow, OnResizeCallback);

	mShaderProgram = BuildShaderProgram("src/Shaders/vertexShader.vert", "src/Shaders/fragmentShader.frag");
	mShaderProgram2 = BuildShaderProgram("src/Shaders/vertexShader.vert", "src/Shaders/fragmentShader2.frag");
	BuildBuffers();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe mode

	return true;
}

GLuint Graphics::Engine::BuildShaderProgram(
	const char* vertexShaderPath,
	const char* fragmentShaderPath
)
{
	Shader vertexShader(vertexShaderPath, GL_VERTEX_SHADER);
	Shader fragmentShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

	GLuint shaderProgram;
	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader.GetID());
	glAttachShader(shaderProgram, fragmentShader.GetID());
	glLinkProgram(shaderProgram);

	GLint programLinkStatus;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &programLinkStatus);

	if (programLinkStatus == GL_FALSE)
	{
		GLint infoLen = 0;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 0)
		{
			char* infoLog = (char*)alloca(infoLen);
			glGetProgramInfoLog(shaderProgram, infoLen, NULL, infoLog);
			std::cout << "Shader program linking error" << std::endl << infoLog << std::endl;
		}

		glDeleteProgram(shaderProgram);
		return 0;
	}

	return shaderProgram;
}

void Graphics::Engine::BuildBuffers()
{
	GLfloat vertices[] = {
		0.5f, 0.5f, 0.0f, // top right
		0.5f, -0.5f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f, // bottom left
		-0.5f, 0.5f, 0.0f // top left
	};

	GLfloat vertices1[] = {
		-0.5f, -0.5f, 0.0f,
		-0.25f, 0.5f, 0.0f,
		0.f, -0.5f, 0.0f,
	};

	GLfloat vertices2[] = {
		0.25f, -0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
		0.75f, -0.5f, 0.0f,
	};

	GLuint indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mEBO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);

	// VAO stores calls to glVertexAttribPointer, glEnableVertexAttribArray, glDisableVertexAttribArray, glBindBuffer for GL_ELEMENT_ARRAY_BUFFER
	glBindVertexArray(mVAO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0); // VBO must be bound before calling this function
	glEnableVertexAttribArray(0);



	glGenVertexArrays(1, &mVAO2);
	glGenBuffers(1, &mVBO2);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

	glBindVertexArray(mVAO2);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);
	glEnableVertexAttribArray(0);


	glBindVertexArray(0); // unbind VAO

	glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind VBO

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind EBO (do it after unbinding VAO otherwise VAO will store unbind call)
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
}

void Graphics::Engine::OnRender()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(mShaderProgram);
	glBindVertexArray(mVAO);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glUseProgram(mShaderProgram2);
	glBindVertexArray(mVAO2);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glfwSwapBuffers(mWindow);
}
