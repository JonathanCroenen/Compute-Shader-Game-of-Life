#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <iomanip>

#include "Shader.h"
#include "ComputeShader.h"

int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 720;

GLFWwindow* window;

double dTime = 0.0;
double lastTime = 0.0;
double timer = 0.0;
int speed = 0;
bool pause = false;
bool drawing = false;

double lastMouseX = 0.0;
double lastMouseY = 0.0;
float panX = 0.0f;
float panY = 0.0f;

glm::mat4 transform;
glm::mat4 translation;
float zoomLevel = 1.0f;
glm::mat4 zoom;

glm::vec4 mousePos;
ComputeShader computeShader;

GLFWwindow* Initialize(int width, int height, const char* title, int vsync);
void SetupBuffers(GLuint& VAO);
void SetupTexture(GLuint width, GLuint height, GLuint& writeTexture, GLuint& readTexture);
void GetComputeGroupInfo();
void KeyBoardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void MouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);


float clamp(float d, float min, float max) {
	const float t = d < min ? min : d;
	return t > max ? max : t;
}
void printMat4(const glm::mat4& m)
{
	std::cout << std::fixed << std::setprecision(2);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout << m[j][i] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl; 
}

void main()
{
	window = Initialize(WINDOW_WIDTH, WINDOW_HEIGHT, "RayTracer", 1);

	Shader shader("C:/Users/jonat/source/repos/ComputeShaders/ComputeShaders/vertex.glsl", "C:/Users/jonat/source/repos/ComputeShaders/ComputeShaders/fragment.glsl");
	computeShader = ComputeShader("C:/Users/jonat/source/repos/ComputeShaders/ComputeShaders/compute.glsl");

	GLuint QuadVAO;
	SetupBuffers(QuadVAO);

	GLuint texWidth = WINDOW_WIDTH, texHeight = WINDOW_HEIGHT;
	GLuint texture0, texture1;
	SetupTexture(texWidth, texHeight, texture0, texture1);

	GetComputeGroupInfo();
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	translation  = glm::mat4(1.0f);
	zoom = glm::mat4(1.0f);

	int currentBuffer = 0;

	while (!glfwWindowShouldClose(window))
	{
		double currentTime = glfwGetTime();
		dTime =  currentTime - lastTime;
		lastTime = currentTime;

		transform = zoom * translation;
		transform[0][0] = clamp(transform[0][0], 1.0f, 8.0f);
		transform[1][1] = clamp(transform[1][1], 1.0f, 8.0f);
		transform[3][0] = clamp(transform[3][0], -transform[0][0], transform[0][0]);
		transform[3][1] = clamp(transform[3][1], -transform[0][0], transform[0][0]);

		mousePos = glm::vec4(2.0f * lastMouseX / WINDOW_WIDTH - 1.0f, -2.0f * lastMouseY / WINDOW_HEIGHT + 1.0f, 0.0f, 1.0f);
		mousePos = glm::inverse(transform) * mousePos;

		timer += dTime ;
		if (timer >= speed * 0.0133333) {
			computeShader.use();
			computeShader.setInt("currentBuffer", currentBuffer);
			computeShader.setInt("pause", pause);
			currentBuffer = !currentBuffer;
			computeShader.dispatch(texWidth, texHeight, 1);
			timer = 0.0f;
		} 

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			shader.use();
			shader.setMat4("transform", transform);
			shader.setInt("state", pause + drawing);

			glBindVertexArray(QuadVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, (currentBuffer && !pause) ? texture0 : texture1);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glfwPollEvents();

			glfwSwapBuffers(window);
	}

	glfwTerminate();
}

GLFWwindow* Initialize(int width, int height, const char* title, int vsync)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL)
	{
		std::cout << "GLFW: failed to create window" << std::endl;
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, KeyBoardCallback);
	glfwSetCursorPosCallback(window, MouseMoveCallback);
	glfwSetScrollCallback(window, ScrollCallback);
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSwapInterval(vsync);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "GLAD: failed to load" << std::endl;
	}

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	return window;
}

void SetupBuffers(GLuint& VAO)
{
	GLfloat quad[] = {
	-1.0f,  1.0f, 0.0f, 1.0f,
	 1.0f,  1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
	 1.0f,  1.0f, 1.0f, 1.0f,
	 1.0f, -1.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, 0.0f, 0.0f
	};

	GLuint VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
}

void SetupTexture(GLuint width, GLuint height, GLuint& texture0, GLuint& texture1)
{
	glGenTextures(1, &texture0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	float* data = new float[4 * width * height];
	//for (int i = 0; i < height; i++) {
	//	for (int j = 0; j < width; j++) {
	//		if (float(rand() % 100) / 100.0f < 0.60f) {
	//			data[4*(j + i * width)] = 0.0f;
	//			data[4 * (j + i * width) + 1] = 0.0f;
	//			data[4 * (j + i * width) + 2] = 0.0f;
	//			data[4 * (j + i * width) + 3] = 1.0f;
	//		}
	//		else {
	//			data[4 * (j + i * width)] = 1.0f;
	//			data[4 * (j + i * width) + 1] = 1.0f;
	//			data[4 * (j + i * width) + 2] = 1.0f;
	//			data[4 * (j + i * width) + 3] = 1.0f;
	//		}
	//	}
	//}
	memset(data, 0.0f, 4 * width * height);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);

	glBindImageTexture(0, texture0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);


	glGenTextures(1, &texture1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glBindImageTexture(1, texture1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
}

void GetComputeGroupInfo()
{
	int workGroupCount[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workGroupCount[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workGroupCount[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workGroupCount[2]);

	std::cout << "Work group count: " << workGroupCount[0] << " " << workGroupCount[1] << " " << workGroupCount[2] << std::endl;

	int workGroupSize[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workGroupSize[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workGroupSize[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workGroupSize[2]);

	std::cout << "Work group size: " << workGroupSize[0] << " " << workGroupSize[1] << " " << workGroupSize[2] << std::endl;

	int workGroupInvocations;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workGroupInvocations);

	std::cout << "Work group invocations: " << workGroupInvocations << std::endl;
}

void KeyBoardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		if (pause && drawing) {
			drawing = !drawing;
		}
		else {
			pause = !pause;
		}
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		if (pause && !drawing) {
			drawing = !drawing;
		}
		else if (pause && drawing) {
			pause = !pause;
			drawing = !drawing;
		}
		else {
			pause = !pause;
			drawing = !drawing;
		}
	}
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		speed++;
		if (speed > 25) speed = 25;
	}
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		speed--;
		if (speed < 1) speed = 1;
	}
}

void MouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
	float offsetx = xpos - lastMouseX;
	float offsety = ypos - lastMouseY;
	lastMouseX = xpos;
	lastMouseY = ypos;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (!drawing) {
			panX += 2 * offsetx / (WINDOW_WIDTH * zoomLevel);
			panY += -2 * offsety / (WINDOW_HEIGHT * zoomLevel);

			panX = clamp(panX, -1.0f, 1.0f);
			panY = clamp(panY, -1.0f, 1.0f);

			translation = glm::translate(glm::mat4(1.0f), glm::vec3(panX, panY, 0.0f));
			computeShader.use();
			computeShader.setVec2("mousePos", -1.0f, -1.0f);
		}
		else if (drawing) {
			computeShader.use();
			computeShader.setVec2("mousePos", WINDOW_WIDTH * (mousePos.x + 1.0f) / 2.0f, WINDOW_HEIGHT * (mousePos.y + 1.0f) / 2.0f);
		}
	}
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{	
	zoomLevel += yoffset * zoomLevel / 10.0f;

	float deltax = 2.0f * lastMouseX / WINDOW_WIDTH - 1.0f;
	float deltay = -2.0f * lastMouseY / WINDOW_HEIGHT + 1.0f;

	glm::mat4 moveTo = glm::translate(glm::mat4(1.0f), glm::vec3(-deltax, -deltay, 0.0f));
	glm::mat4 moveBack = glm::translate(glm::mat4(1.0f), glm::vec3(deltax, deltay, 0.0f));

	if (zoomLevel > 8.0f)
	{
		zoomLevel = 8.0f;
		glm::mat4 moveTo = glm::mat4(1.0f);
		glm::mat4 moveBack = glm::mat4(1.0f);
	}

	if (zoomLevel < 1.0f)
	{
		zoomLevel = 1.0f;
		glm::mat4 moveTo = glm::mat4(1.0f);
		glm::mat4 moveBack = glm::mat4(1.0f);
	}

	zoom = glm::scale(glm::mat4(1.0f), glm::vec3(zoomLevel, zoomLevel, 1.0f));
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	WINDOW_WIDTH = width;
	WINDOW_HEIGHT = height;
}