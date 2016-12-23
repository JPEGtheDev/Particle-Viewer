/*
* clutter.hpp
*
* This is the 'Junk' hpp file that contains any and all loose functions in the
* main loop as well as all includes for the main
*
*/

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
	void init_screen(const char * caption);
	void upDeltaTime();
	void beforeDraw();
	void drawFunct();
	void setupGLStuff();
	void cleanup();
	void setupRender();
	void seekFrame(int frame, bool isForward);
	void calculateTime(long long frame, float dt, float recordRate, float unitTime);
	GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencil);
	static void key_callback(GLFWwindow*,int,int,int,int);
	void setupScreenFBO ();
	void drawFBO();
//variables
	const GLint
		SCREEN_FULLSCREEN = 0,
		SCREEN_WIDTH  = 1280,
		SCREEN_HEIGHT = 720;

	GLboolean quit = false, isRecording = false;
	GLint
		curFrame = 0,		// Current frame number (must be > 0)
		imageError = 0,		// Number of errors when trying to save a frame
		imageErrorMax = 5;	// The maximum number of errors when trying to save a frame
	GLfloat
		sphereScale = 1.0,		// The scale of the spheres.
		sphereRadius = 250.0f,	// Radius of the spheres rendered.
		deltaTime = 0.0f,		// Time it took to render the last frame.
		lastFrame = 0.0f;		// Timestamp of the last frame.

	GLuint
		quadVAO,			// Array Object for the screen.
		quadVBO,			// Buffer Object for the screen.
		framebuffer,		// Holds screen data for the spheres.
		rbo,				// Renderbuffer for the spheres.
		textureColorbuffer,	// Holds color data for the spheres.
		circleVAO,			// Sphere VAO
		circleVBO;			// Sphere VBO

	Camera cam = Camera(SCREEN_WIDTH,SCREEN_HEIGHT);
	GLFWwindow *window = NULL;
	glm::mat4 view;
	glm::vec3 com;			// Center of mass for the simulation.
	Particle* part;
	SettingsIO *set = new SettingsIO();
	Shader sphereShader, screenShader;

	std::string exePath;
	std::string sphereVertexShader = "/Viewer-Assets/shaders/sphereVertex.vs";
	std::string sphereFragmentShader = "/Viewer-Assets/shaders/sphereFragment.frag";
	std::string screenVertexShader = "/Viewer-Assets/shaders/screenshader.vs";
	std::string screenFragmentShader = "/Viewer-Assets/shaders/screenshader.frag";
	std::string recordFolder = "";

	unsigned char * pixels = new unsigned char[SCREEN_WIDTH*SCREEN_HEIGHT*3];
	unsigned char * pixels2 = new unsigned char[SCREEN_WIDTH*SCREEN_HEIGHT*3];
//functions that should not be changed

	/*
	* Updates deltaTime
	*/
	void upDeltaTime()
	{
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
	}

	/*
	* Allows GLFW to output any errors to the console
	*/
	void error_callback(int error, const char* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	}

	/*
	* Initializes the GLFW window with a title
	*/
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
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);		//Allows rendering on OSX
		#endif

		window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title, NULL, NULL);
		if (!window){}
		glfwSetKeyCallback(window, key_callback);
		glfwMakeContextCurrent(window);
		gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);			//Links GLAD to GLFW
		glfwSwapInterval(1);
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}

	/*
	* Initializes the paths to the executable.
	* This is used to link external files, such as fonts or shaders to the application at runtime.
	*/
	void initPaths()
	{
		exePath = ExePath();
		sphereVertexShader = exePath + sphereVertexShader;
		sphereFragmentShader = exePath + sphereFragmentShader;
		screenVertexShader = exePath + screenVertexShader;
		screenFragmentShader = exePath + screenFragmentShader;
	}

	/*
	* Sets up keys
	*/
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
			seekFrame(1,true);
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		{
			seekFrame(1,false);
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

	/*
	* Generates the texture for the screen Frame Buffer Object.
	*
	* Pulled from https://learnopengl.com/#!Advanced-OpenGL/Framebuffers
	*/
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		return textureID;
	}
	GLfloat quadVertices[] =
	{   // Vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // Positions   // TexCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

	/*
	* Calculates simulation time based off of the frame number.
	*/
	void calculateTime(long long frame, float dt, float recordRate, float unitTime)
	{
		double calc = frame * dt * recordRate * unitTime;
		int seconds = fmod(calc, 60);
		int minutes = fmod((calc / 60), 60);
		int hours	= fmod((calc / 3600), 24);
		int days	= fmod((calc / 86400), 365);
		std::string secondS = ((seconds < 10) ? "0" + std::to_string(seconds) : std::to_string(seconds));
		std::string minuteS = ((minutes < 10) ? "0" + std::to_string(minutes) : std::to_string(minutes));
		std::string hourS   = ((hours   < 10) ? "0" + std::to_string(hours  ) : std::to_string(hours));

		std::cout << "\nReal Time: " << days << " D | " << hourS << " H | " << minuteS << " M | " << secondS << " S" << std::endl;
	}

	/*
	* Sets up the screen Frame Buffer Object (quadVAO / quadVAO) to pull data from the primary framebuffer.
	*
	* Pulled from https://learnopengl.com/#!Advanced-OpenGL/Framebuffers
	*/
	void setupScreenFBO ()
	{
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
		glBindVertexArray(0);

		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		// Create a color attachment texture
		textureColorbuffer = generateAttachmentTexture(false, false);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT); // Use a single renderbuffer object for both a depth AND stencil buffer.
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // Now actually attach it
		// Now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	/*
	* Draws the screen Frame Buffer Object (quadVAO / quadVAO) to the screen.
	*
	* Pulled from https://learnopengl.com/#!Advanced-OpenGL/Framebuffers
	*/
	void drawFBO()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// Clear all relevant buffers
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set clear color to white (not really necessery actually, since we won't be able to see behind the quad anyways)
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST); // We don't care about depth information when rendering a single quad
			// Draw Screen
		screenShader.Use();
		glBindVertexArray(quadVAO);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);	// Use the color attachment texture as the texture of the quad plane
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

