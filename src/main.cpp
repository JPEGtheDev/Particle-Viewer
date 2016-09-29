#include "clutter.hpp"
#include <openvr.h>
vr::IVRSystem* hmd;
vr::HmdError* vrError;
vr::IVRCompositor* compositor;
vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
#   pragma comment(lib, "openvr_api")

std::string getHMDString(vr::IVRSystem* pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* peError = nullptr) {
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, nullptr, 0, peError);
	if (unRequiredBufferLen == 0) {
		return "";
	}

	char* pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;

	return sResult;
}

vr::IVRSystem* initOpenVR(uint32_t& hmdWidth, uint32_t& hmdHeight) {
	vr::EVRInitError eError = vr::VRInitError_None;
	vr::IVRSystem* hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);

	if (eError != vr::VRInitError_None) {
		fprintf(stderr, "OpenVR Initialization Error: %s\n", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		return nullptr;
	}

	const std::string& driver = getHMDString(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	const std::string& model = getHMDString(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ModelNumber_String);
	const std::string& serial = getHMDString(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);
	const float freq = hmd->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);

	//get the proper resolution of the hmd
	hmd->GetRecommendedRenderTargetSize(&hmdWidth, &hmdHeight);

	fprintf(stderr, "HMD: %s '%s' #%s (%d x %d @ %g Hz)\n", driver.c_str(), model.c_str(), serial.c_str(), hmdWidth, hmdHeight, freq);

	// Initialize the compositor
	vr::IVRCompositor* compositor = vr::VRCompositor();
	if (!compositor) {
		fprintf(stderr, "OpenVR Compositor initialization failed. See log file for details\n");
		vr::VR_Shutdown();
		assert("VR failed" && false);
	}

	return hmd;
}
uint32_t framebufferWidth = 1280, framebufferHeight = 720;
int main(int argc, char* argv[])
{
	
	const int numEyes = 2;
	hmd = initOpenVR(framebufferWidth, framebufferHeight);

	initPaths();
	init_screen("Particle-Viewer");
	cam.initGL();
	part = new Particle();
	setupGLStuff();
	setupScreenFBO();


	vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	while (!glfwWindowShouldClose(window)) 
	{
		vr::VRCompositor()->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
		glfwPollEvents();
		cam.Move();
		beforeDraw();
		drawFunct();
		cam.RenderSphere(); 
		drawFBO();
		//render GUI
		
		const vr::Texture_t tex = { reinterpret_cast<void*>(intptr_t(textureColorbuffer)), vr::API_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::EVREye(0), &tex);
		vr::VRCompositor()->Submit(vr::EVREye(1), &tex);
		
		glfwSwapBuffers(window);
		vr::VRCompositor()->PostPresentHandoff();
		
		if(set->frames > 1)
		{
			set->readPosVelFile(curFrame,part,false);
		}
		
		if(set->isPlaying)
		{
			if(curFrame != 0)
			{
				calculateTime(curFrame, set->getDt(), 50.0, 2935.864063);
			}
			curFrame++;

		}
		if(curFrame > set->frames)
		{
			curFrame = set->frames;
		}
		if (curFrame < 0)
		{
			curFrame = 0;
		}
	}
	cleanup();
	return 0;
}
void beforeDraw()
{
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	cam.update(deltaTime);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	upDeltaTime();
	view = cam.setupCam();
	
	
}
void drawFunct()
{
	set->getCOM(curFrame, com);
	cam.setSphereCenter(com);
	sphereShader.Use();
	part->pushVBO();
	glBindVertexArray(circleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, part->instanceVBO);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUniformMatrix4fv(glGetUniformLocation(sphereShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(sphereShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(cam.projection));
	glUniform1f(glGetUniformLocation(sphereShader.Program, "radius"), sphereRadius);
	glUniform1f(glGetUniformLocation(sphereShader.Program, "scale"), sphereScale);
	glDrawArraysInstanced(GL_POINTS,0,1,part->n);
	glBindVertexArray(0);
	//take screenshot
	if(set->isPlaying && isRecording)
	{
		glReadPixels(0,0,(int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		for (int i = 0; i < SCREEN_WIDTH * 3; i++)
		{
			for(int j = 0; j < SCREEN_HEIGHT; j++)
			{
				pixels2[i + SCREEN_WIDTH* 3 * j] = pixels[i+ SCREEN_WIDTH* 3 * (SCREEN_HEIGHT - j)];
			}
		}
		//
		if(!stbi_write_tga(std::string(recordFolder+"/" + std::to_string(curFrame) + ".tga").c_str(), (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, 3, pixels2))
		{
			if(imageError < 5)
			{
				imageError++;
				std::cout << "Unable to save image: Error "<< imageError << std::endl;
			}
			else
			{
				std::cout << "Max Image Error Count Reached! Ending Recording!"<< std::endl;
				isRecording = false;
			}
		}
	}
	
}

void setupGLStuff() 
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE );
	glEnable(GL_MULTISAMPLE);  
	sphereShader = Shader(sphereVertexShader.c_str(),sphereFragmentShader.c_str());							//creates the shader to be used on the spheres
	screenShader = Shader(screenVertexShader.c_str(),screenFragmentShader.c_str());
	glGenVertexArrays(1, &circleVAO);
	glGenBuffers(1, &circleVBO);
	glBindVertexArray(circleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
	
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	part->setUpInstanceArray();
	glBindVertexArray(0);
	//set up other object arrays
}
void seekFrame(int frame, bool isForward)
{
	if(isForward)
	{
		curFrame += frame;
	}
	else
	{
		curFrame -= frame;
	}
	
}
void cleanup()
{
	delete part;
	delete[] pixels;
	delete[] pixels2;
	glDeleteFramebuffers(1, &framebuffer);
}
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
