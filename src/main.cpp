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
//1512
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
uint32_t framebufferWidth = 2560, framebufferHeight = 1680;

void getEyeTransformations
(vr::IVRSystem*  hmd,
	vr::TrackedDevicePose_t* trackedDevicePose,
	float           nearPlaneZ,
	float           farPlaneZ,
	glm::mat4       &headToWorldRowMajor3x4,
	glm::mat4       &ltEyeToHeadRowMajor3x4,
	glm::mat4       &rtEyeToHeadRowMajor3x4,
	glm::mat4       &ltProjectionMatrixRowMajor4x4,
	glm::mat4       &rtProjectionMatrixRowMajor4x4)
{

	assert(nearPlaneZ < 0.0f && farPlaneZ < nearPlaneZ);

	vr::VRCompositor()->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
	assert(trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid);
	const vr::HmdMatrix34_t head = trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking;
	const vr::HmdMatrix44_t& ltProj = hmd->GetProjectionMatrix(vr::Eye_Left, -nearPlaneZ, -farPlaneZ, vr::API_OpenGL);
	const vr::HmdMatrix44_t& rtProj = hmd->GetProjectionMatrix(vr::Eye_Right, -nearPlaneZ, -farPlaneZ, vr::API_OpenGL);
	const vr::HmdMatrix34_t& ltMatrix = hmd->GetEyeToHeadTransform(vr::Eye_Left);
	const vr::HmdMatrix34_t& rtMatrix = hmd->GetEyeToHeadTransform(vr::Eye_Right);
	for (int r = 0; r < 3; ++r) 
	{
		for (int c = 0; c < 4; ++c) 
		{
			ltEyeToHeadRowMajor3x4[c][r] = ltMatrix.m[r][c];
			rtEyeToHeadRowMajor3x4[c][r] = rtMatrix.m[r][c];
			headToWorldRowMajor3x4[c][r] = head.m[r][c];
		}
	}
	for (int r = 0; r < 4; ++r)
	{
		for (int c = 0; c < 4; ++c)
		{
			ltProjectionMatrixRowMajor4x4[c][r] = ltProj.m[r][c];
			rtProjectionMatrixRowMajor4x4[c][r] = rtProj.m[r][c];
		}
	}
}
glm::vec3 bodyRotation;
const float verticalFieldOfView = 45.0f * glm::pi<float>() / 180.0f;

int main(int argc, char* argv[])
{
	
	const int numEyes = 2;
	hmd = initOpenVR(framebufferWidth, framebufferHeight);

	initPaths();
	init_screen("Particle-Viewer");
	part = new Particle();
	setupGLStuff();
	setupScreenFBO();


	vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	while (!glfwWindowShouldClose(window)) 
	{
		glfwPollEvents();
		cam.Move();
		const float nearPlaneZ = -0.1f;
		const float farPlaneZ = -1000.0f;
		
		glm::mat4 eyeToHead[numEyes], projectionMatrix[numEyes], headToBodyMatrix;
		getEyeTransformations(hmd, trackedDevicePose, nearPlaneZ, farPlaneZ, headToBodyMatrix, eyeToHead[0], eyeToHead[1], projectionMatrix[0], projectionMatrix[1]);
		
		glm::mat4 translate; // bodyTranslation
		translate[0] = glm::vec4(1.0f, 0, 0, 0);
		translate[1] = glm::vec4(0, 1.0f, 0, 0);
		translate[2] = glm::vec4(0, 0, 1.0f, 0);
		translate[3] = glm::vec4(bodyTranslation.x, bodyTranslation.y, bodyTranslation.z, 1.0f);
		glm::mat4 roll;
		roll[0] = glm::vec4(cos(bodyRotation.z), sin(bodyRotation.z), 0, 0);
		roll[1] = glm::vec4(-sin(bodyRotation.z), cos(bodyRotation.z), 0, 0);
		roll[2] = glm::vec4(0, 0, 1.0f, 0);
		roll[3] = glm::vec4(0, 0, 0, 1.0f);
		glm::mat4 yaw;
		yaw[0] = glm::vec4(cos(bodyRotation.z), 0, -sin(bodyRotation.z), 0);
		yaw[1] = glm::vec4(0, 1.0f, 0, 0);
		yaw[2] = glm::vec4(sin(bodyRotation.z), 0, cos(bodyRotation.z), 0);
		yaw[3] = glm::vec4(0, 0, 0, 1.0f);
		glm::mat4 pitch;

		pitch[0] = glm::vec4(1.0f, 0, 0, 0);
		pitch[1] = glm::vec4(0, cos(bodyRotation.z), sin(bodyRotation.z), 0);
		pitch[2] = glm::vec4(0, -sin(bodyRotation.z), cos(bodyRotation.z), 0);
		pitch[3] = glm::vec4(0, 0, 0, 1.0f);
		const glm::mat4& bodyToWorldMatrix =
			translate * roll* yaw * pitch;
		const glm::mat4& headToWorldMatrix = bodyToWorldMatrix * headToBodyMatrix;

		for (int i = 0; i < 3; i++)
		{
			
			beforeDraw(i,headToWorldMatrix,eyeToHead,projectionMatrix);
			drawFunct(i, projectionMatrix);
			//const vr::VRTextureBounds_t* a = &(vr::VRTextureBounds_t{ 0.0f,0.0f,0.5f,1.0f });
			//const vr::VRTextureBounds_t* b = &(vr::VRTextureBounds_t{ 0.5f,0.0f,1.0f,1.0f });
			const vr::Texture_t tex = { reinterpret_cast<void*>(intptr_t(textureColorbuffer)), vr::API_OpenGL, vr::ColorSpace_Gamma };
			vr::VRCompositor()->Submit(vr::EVREye(i), &tex);
		}
		
		//vr::VRCompositor()->Submit(vr::EVREye(1), &tex);
		drawFBO();
		//render GUI
		
		
		
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
void beforeDraw(int i,glm::mat4 headToWorldMatrix, glm::mat4 eyeToHead[2],glm::mat4 projectionMatrix[2])
{
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glm::mat4 yaw;
	yaw[0] = glm::vec4(cos(glm::pi<float>() / 3.0f), 0, -sin(glm::pi<float>() / 3.0f), 0);
	yaw[1] = glm::vec4(0, 1.0f, 0, 0);
	yaw[2] = glm::vec4(sin(glm::pi<float>() / 3.0f), 0, cos(glm::pi<float>() / 3.0f), 0);
	yaw[3] = glm::vec4(0, 0, 0, 1.0f);
	glm::mat4 translate;
	translate[0] = glm::vec4(1.0f, 0, 0, 0);
	translate[1] = glm::vec4(0, 1.0f, 0, 0);
	translate[2] = glm::vec4(0, 0, 1.0f, 0);
	translate[3] = glm::vec4(0, .5f, 0, 1.0f);
	const glm::mat4& objectToWorldMatrix = translate * yaw;
	glm::mat4& cameraToWorldMatrix = headToWorldMatrix * eyeToHead[i%2];
	const glm::mat3& objectToWorldNormalMatrix = glm::inverse(glm::transpose(glm::mat3(objectToWorldMatrix)));
	const glm::mat4& modelViewProjectionMatrix = glm::inverse(cameraToWorldMatrix) * objectToWorldMatrix;
	
	view = modelViewProjectionMatrix;
		
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(i == 0)
	{
		cam.update(deltaTime);
		upDeltaTime();
	}
}
void drawFunct(int i,glm::mat4 projectionMatrix[2])
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
	glUniformMatrix4fv(glGetUniformLocation(sphereShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix[i%2]));
	glUniform1f(glGetUniformLocation(sphereShader.Program, "radius"), sphereRadius);
	glUniform1f(glGetUniformLocation(sphereShader.Program, "scale"), sphereScale);
	glUniform1f(glGetUniformLocation(sphereShader.Program, "transScale"),sphereScale/4);
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
