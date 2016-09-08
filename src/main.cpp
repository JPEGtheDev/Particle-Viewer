#include "clutter.hpp"

int main(int argc, char* argv[])
{

	initPaths();
	init_screen("Particle-Viewer");
	cam.initGL();
	part = new Particle();
	setupGLStuff();
	setupText();


	while (!glfwWindowShouldClose(window)) 
	{
		glfwPollEvents();
		cam.Move();
		//readInput(event);
		beforeDraw();
		drawFunct();
		glfwSwapBuffers(window);
        
		
		if(set->frames > 1)
		{
			set->readPosVelFile(curFrame,part,false);
		}
		
		if(set->isPlaying)
		{
			if(curFrame != 0)
			{
				
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
	glClear(GL_DEPTH_BUFFER_BIT);
	RenderText(textShader, calculateTime(curFrame, set->getDt(), 50.0, 2935.864063), 25.0f, 25.0f, .5f, glm::vec3(1, 1, 1));
	RenderText(textShader, "Tarleton State University", 900.0f, 25.0f, .5f, glm::vec3(1, 1, 1));
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
	//draw GUI elements
	cam.RenderSphere();
}

void setupGLStuff() 
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE );
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	sphereShader = Shader(sphereVertexShader.c_str(),sphereFragmentShader.c_str());							//creates the shader to be used on the spheres

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
}
std::string calculateTime(long long frame, float dt, float recordRate, float unitTime)
{
	double calc = frame * dt * recordRate * unitTime;
	int seconds = fmod(calc, 60);
	int minutes = fmod((calc / 60), 60);
	int hours	= fmod((calc / 3600), 24);
	int days	= fmod((calc / 86400), 365);
	std::string secondS = ((seconds < 10) ? "0" + std::to_string(seconds) : std::to_string(seconds));
	std::string minuteS = ((minutes < 10) ? "0" + std::to_string(minutes) : std::to_string(minutes));
	std::string hourS   = ((hours   < 10) ? "0" + std::to_string(hours  ) : std::to_string(hours));

	return "Real Time: " + std::to_string(days) + " D | " + hourS + " H | " + minuteS + " M | " + secondS + " S";
}

void setupText()
{
	textShader = Shader(textVertexShader.c_str(), textFragmentShader.c_str());
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(SCREEN_WIDTH), 0.0f, static_cast<GLfloat>(SCREEN_HEIGHT));
    textShader.Use();
    glUniformMatrix4fv(glGetUniformLocation(textShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
	if (FT_Init_FreeType(&ft))
	    std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	FT_Face face;
	if (FT_New_Face(ft, (fontFolder + "hack.ttf").c_str(), 0, &face))
	    std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
	 FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void RenderText(Shader &shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    // Activate corresponding render state	
	textShader.Use();
    glUniform3f(glGetUniformLocation(shader.Program, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}