//includes
	#include <iostream>
	#include <string>
	#include "math.h"
	#include "glm/glm.hpp"
	#include "glm/gtc/matrix_transform.hpp"
	#include "glm/gtc/type_ptr.hpp"
	#include "glad/glad.h"
	#include "shader.hpp"
	#include "camera.hpp"
	#include "tinyFileDialogs/tinyfiledialogs.h"
	#include "particle.hpp"
	#include "settingsIO.hpp"
	#define STB_IMAGE_WRITE_IMPLEMENTATION
	#include "stb/stb_image_write.h"
	#include <GLFW/glfw3.h>
	#include "osFile.hpp"
	#ifdef _WIN32 //Windows Includes
		#include <windows.h>
	#endif
//function prototypes
	void init_screen(const char * caption);					//initializes the screen / window / context
	void upDeltaTime();										//updates deltaTime
	void beforeDraw(int,glm::mat4,glm::mat4[2],glm::mat4[2]);									//the generic clear screen and other stuff before anything else has to be done
	void drawFunct(int,glm::mat4[2]); 							//Draws stuff to the screen
	void setupGLStuff();									//sets up the VAOs and the VBOs
	void cleanup();											//destroy it all with fire
	void setupRender();										//Updates the VBOs for position changes
	void seekFrame(int frame, bool isForward);				//skips frames
	void calculateTime(long long frame, float dt, float recordRate, float unitTime);
	GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencil); 
	static void key_callback(GLFWwindow*,int,int,int,int);	//key commands for GLFW
	void setupScreenFBO ();
	void drawFBO();
//variables
	glm::vec3 bodyTranslation(0.0f, 1.6f, 5.0f);
	const int SCREEN_FULLSCREEN = 0, SCREEN_WIDTH  = 1512, SCREEN_HEIGHT = 1680;
	int curFrame = 0;
	bool quit = false, highRes = false;
	float sphereScale = .25f;
	float sphereRadius = 250.0f;
	bool isRecording = false;
	GLuint circleVAO, circleVBO;
	std::string recordFolder = "";
	GLfloat deltaTime = 0.0f, lastFrame = 0.0f;
	int imageError = 0;
	GLFWwindow *window = NULL;
	glm::vec3 com;
	unsigned char * pixels = new unsigned char[SCREEN_WIDTH*SCREEN_HEIGHT*3];
	unsigned char * pixels2 = new unsigned char[SCREEN_WIDTH*SCREEN_HEIGHT*3];
	Shader sphereShader, screenShader;
	std::string exePath;
	Camera cam = Camera(SCREEN_WIDTH,SCREEN_HEIGHT);
	Particle* part;
	glm::mat4 view;
	GLuint quadVAO, quadVBO;
	GLuint framebuffer;

	GLuint rbo;
	GLuint textureColorbuffer;
	std::string sphereVertexShader = "/Viewer-Assets/shaders/sphereVertex.vs";
	std::string sphereFragmentShader = "/Viewer-Assets/shaders/sphereFragment.frag";
	std::string screenVertexShader = "/Viewer-Assets/shaders/screenshader.vs";
	std::string screenFragmentShader = "/Viewer-Assets/shaders/screenshader.frag";
	SettingsIO *set = new SettingsIO();  
//functions that should not be changed
	void upDeltaTime()
	{
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
	}
	void error_callback(int error, const char* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	}
	void init_screen(const char * title) 
	{
		glfwSetErrorCallback(error_callback);
		if (!glfwInit())
		exit(EXIT_FAILURE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		#ifdef __APPLE__
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		#endif
		//creates the window

		window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title, NULL, NULL);
		if (!window)
		{

		}
		glfwSetKeyCallback(window, key_callback);
		glfwMakeContextCurrent(window);
		gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
		glfwSwapInterval(1);
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}

	void initPaths() 
	{	
		exePath = ExePath();
		sphereVertexShader = exePath + sphereVertexShader;
		sphereFragmentShader = exePath + sphereFragmentShader;
		screenVertexShader = exePath + screenVertexShader;
		screenFragmentShader = exePath + screenFragmentShader;
	}

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		cam.KeyReader(window,key,scancode,action,mods);
		if (key == GLFW_KEY_Q && action == GLFW_REPEAT)
		{
			seekFrame(3, false);
		}
		if (key == GLFW_KEY_E && action == GLFW_REPEAT)
		{
			seekFrame(3, true);
		}
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GLFW_TRUE);

		}
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		{
			set->togglePlay();
		}
		if (key == GLFW_KEY_T && action == GLFW_PRESS)
		{
			set = set->loadFile(part,false);
			curFrame = 0;
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		{
			seekFrame(1, true);
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		{
			seekFrame(1,false);
		}
		if (key == GLFW_KEY_UP)
		{
			
			sphereScale *= 1.5f;
			std::cout << "Sphere Scale: " << sphereScale << std::endl;
		}
		if (key == GLFW_KEY_DOWN)
		{
			sphereScale /= 1.5f;
			
			std::cout << "Sphere Scale: " << sphereScale << std::endl;
		}
		if (key == GLFW_KEY_W)
		{
			bodyTranslation.z -= 1;

			std::cout << "Sphere Scale: " << sphereScale << std::endl;
		}
		if (key == GLFW_KEY_R && action == GLFW_PRESS)
		{
			if(!isRecording)
			{
				imageError = 0;
				std::string dialog = "Select Folder";
				const char* fol = tinyfd_selectFolderDialog (dialog.c_str() , "") ;
				
				std::string folder;
				if(fol != NULL)
				{
					folder = std::string(fol);
				}
				else
				{
					folder = "";
				}
				if(folder != "")
				{
					recordFolder = folder;
					isRecording = true;
					return;
				}
				std::cout << "Folder not selected" << std::endl;
				isRecording = false;
			}
			else
			{
				recordFolder = "";
				isRecording = false;
			}
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		{
			seekFrame(1,false);
		}
	}
	GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencil)
	{
		// What enum to use?
		GLenum attachment_type;
		if(!depth && !stencil)
			attachment_type = GL_RGB;
		else if(depth && !stencil)
			attachment_type = GL_DEPTH_COMPONENT;
		else if(!depth && stencil)
			attachment_type = GL_STENCIL_INDEX;

		//Generate texture ID and load texture data 
		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		if(!depth && !stencil)
			glTexImage2D(GL_TEXTURE_2D, 0, attachment_type, SCREEN_WIDTH, SCREEN_HEIGHT, 0, attachment_type, GL_UNSIGNED_BYTE, NULL);
		else // Using both a stencil and depth test, needs special format arguments
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		return textureID;
	}
	GLfloat quadVertices[] = {   // Vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // Positions   // TexCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };	
