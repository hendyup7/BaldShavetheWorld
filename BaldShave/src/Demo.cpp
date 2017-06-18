#include "Enemy.h"

Enemy enemy[4];

Demo::Demo()
{
}

Demo::~Demo()
{
}

void Demo::Init()
{
	BuildBG();
	BuildSprite();
	BuildWall(); 
	BuildSword();
	InitAndPlayMusic();
	InitFont();

	for (int i = 0; i < 4; i++)
		enemy[i].Init(2.0f, 2.0f);

	srand(1);

}

void Demo::Update(float deltaTime)
{
	UpdateSprite(deltaTime);
	MoveSprite(deltaTime);
	if (death)
	{
		QuitHandler(deltaTime);
	}
	
	CheckCollide();
	
	EnemySpawn(deltaTime);
	for (int i = 0; i < 4; i++)
		if (!enemy[i].getStatus())
			enemy[i].Update(deltaTime, getPPosx(), getPPosy());

}

void Demo::Render()
{
	//Setting Viewport
	glViewport(0, 0, screenWidth, screenHeight);

	//Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Set the background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	DrawWall();
	DrawBG();
	DrawSprite();
	DrawSword();
	for (int i = 0; i < 4; i++) {
		if (!enemy[i].getStatus())
			enemy[i].Render();

	}	
	
	mat4 projection = ortho(0.0f, (float)this->screenWidth, 0.0f, (float)this->screenHeight);
	GLint fontlocation = glGetUniformLocation(this->fontProgram, "projection");
	UseShader(this->fontProgram);
	glUniformMatrix4fv(fontlocation, 1, GL_FALSE, value_ptr(projection));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	string scr = to_string(score);
	
	RenderText("Score", 650.0f, 60.0f, 1.0f, vec3(1.0f, 1.0f, 1.0f));
	RenderText("W A S D     to     Move", 50.0f, 580.0f, 1.0f, vec3(1.0f, 1.0f, 1.0f));
	RenderText("HOLD     SPACE    to     Attack", 530.0f, 580.0f, 1.0f, vec3(1.0f, 1.0f, 1.0f));
	RenderText(scr, 650.0f, 35.0f, 1.0f, vec3(1.0f, 1.0f, 1.0f));
	glDisable(GL_BLEND);

}

void Demo::QuitHandler(float deltaTime) {
	countDown += deltaTime;

	if (countDown > 2000)
	{
		SDL_Quit();
		setState(State::EXIT);
	}
}

void Demo::InitFont() {
	// Init Freetype
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	FT_Face face;
	if (FT_New_Face(ft, FONTNAME, 0, &face))
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

	FT_Set_Pixel_Sizes(face, 0, FONTSIZE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
	for (GLubyte c = 0; c < 128; c++)
	{
		// Load character glyph
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// Generate texture
		GLuint fontTexture;
		glGenTextures(1, &fontTexture);
		glBindTexture(GL_TEXTURE_2D, fontTexture);
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
			fontTexture,
			ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(pair<GLchar, Character>(c, character));
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	glGenVertexArrays(1, &fontVAO);
	glGenBuffers(1, &fontVBO);
	glBindVertexArray(fontVAO);
	glBindBuffer(GL_ARRAY_BUFFER, fontVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL,
		GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	this->fontProgram = BuildShader("resource/font.vert", "resource/font.frag");
}

void Demo::RenderText(string text, GLfloat x, GLfloat y, GLfloat scale, vec3 color)
{
	// Activate corresponding render state
	UseShader(this->fontProgram);
	glUniform3f(glGetUniformLocation(this->fontProgram, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(fontVAO);

	// Iterate through all characters
	string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];
		GLfloat xtpos = x + ch.Bearing.x * scale;
		GLfloat ytpos = y - (ch.Size.y - ch.Bearing.y) * scale;
		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xtpos, ytpos + h, 0.0, 0.0 },
			{ xtpos, ytpos, 0.0, 1.0 },
			{ xtpos + w, ytpos, 1.0, 1.0 },
			{ xtpos, ytpos + h, 0.0, 0.0 },
			{ xtpos + w, ytpos, 1.0, 1.0 },
			{ xtpos + w, ytpos + h, 1.0, 0.0 }
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, fontVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1 / 64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels(2 ^ 6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Demo::InitAndPlayMusic() {
	ISoundEngine *SoundEngine = createIrrKlangDevice();
	SoundEngine->play2D(this->MP3NAME, GL_TRUE);
}

void Demo::ScoreCalc() {
	score += 100;
	if (score == 1000) {
		spawnDelay = 2500;
		for (int i = 0; i < 4; i++)
			enemy[i].setSpeed(0.000125f);
		
	}

	if (score == 5000) {
		spawnDelay = 1500;
		for (int i = 0; i < 4; i++)
			enemy[i].setSpeed(0.0002f);

	}
		

}

void Demo::EnemySpawn(float deltaTime) {

	if (enemy[0].getStatus() || enemy[1].getStatus() || enemy[2].getStatus() || enemy[3].getStatus()) {
		spawnDelta += deltaTime;

		if (spawnDelta > spawnDelay) {
			int index = enemySpawned % 4;

			while (!enemy[index].getStatus()) {
				index++;
				index %= 4;

			}

			if (index == 0)
				enemy[index].Init(RandomNumber(-0.9f, -0.8f), RandomNumber(-0.9f, -0.8f));
			if (index == 1)
				enemy[index].Init(RandomNumber(-0.9f, -0.8f), RandomNumber(0.8f, 0.9f));
			if (index == 2)
				enemy[index].Init(RandomNumber(0.8f, 0.9f), RandomNumber(0.8f, 0.9f));
			if (index == 3)
				enemy[index].Init(RandomNumber(0.8f, 0.9f), RandomNumber(-0.9f, -0.8f));
			enemy[index].setStatus(false);

			spawnDelta = 0;
			enemySpawned++;

		}

	}


}

void Demo::UpdateSprite(float deltaTime)
{
	// Update animation
	frame_dur += deltaTime;

	if (walk_anim && frame_dur > FRAME_DUR) {
		frame_dur = 0;
		if (frame_idx == NUM_FRAMES - 1) frame_idx = 0;  else frame_idx++;

		// Pass frameIndex to shader
		GLint location = glGetUniformLocation(this->program, "frameIndex");
		UseShader(this->program);
		glUniform1i(location, frame_idx);
	}
	else if (hitting && frame_dur > FRAME_DUR) {
		frame_dur = 0;
		if (frame_idx == NUM_FRAMES2 - 1) {
			frame_idx = 0;
			hitting = false;
		}
		else frame_idx++;

		// Pass frameIndex to shader
		GLint location = glGetUniformLocation(this->program, "frameIndex");
		UseShader(this->program);
		glUniform1i(location, frame_idx);

		// Pass frameIndex to shader
		GLint swordlocation = glGetUniformLocation(this->swordprogram, "frameIndex");
		UseShader(this->swordprogram);
		glUniform1i(swordlocation, frame_idx);
	}
	else if (death && frame_dur > FRAME_DUR) {
		frame_dur = 0;
		if (frame_idx == NUM_FRAMES2 - 1) frame_idx == NUM_FRAMES2 - 1;  else frame_idx++;

		// Pass frameIndex to shader
		GLint location = glGetUniformLocation(this->program, "frameIndex");
		UseShader(this->program);
		glUniform1i(location, frame_idx);
	}
}

void Demo::MoveSprite(float deltaTime)
{
	walk_anim = false;
	
	if (IsKeyDown(SDLK_SPACE) && !hitting && !death)
	{
		//PlaySound("C:\\Users\\ASUS A46CM\\Downloads\\Pemrograman_Game\\Pemrograman_Game\\vs\\Lesson05\\hit_2.wav", NULL, SND_FILENAME | SND_ASYNC);
		if (horizontal == 3)
		{
			horizontal = 5;
			xspos = xpos;
			yspos = ypos;
			frame_idx = 0;
			BuildSprite();
			BuildSword();
		}
		else if (horizontal == 4) {
			horizontal = 6;
			xspos = xpos;
			yspos = ypos;
			frame_idx = 0;
			BuildSprite();
			BuildSword();
		}
		else if (horizontal == 1)
		{
			horizontal = 7;
			xspos = xpos;
			yspos = ypos;
			frame_idx = 0;
			BuildSprite();
			BuildSword();
		}
		else if (horizontal == 2)
		{
			horizontal = 8;
			xspos = xpos;
			yspos = ypos;
			frame_idx = 0;
			BuildSprite();
			BuildSword();
		}
		hitting = true;
	}
	if (IsKeyDown(SDLK_d) && !hitting && !IsKeyDown(SDLK_a) && !IsKeyDown(SDLK_w) && !IsKeyDown(SDLK_s) && !death) {
		if (horizontal != 1)
		{
			horizontal = 1;
			BuildSprite();
		}
		if (xpos < 0.85f)
			xpos += deltaTime * speed;
		walk_anim = true;
	}
	if (IsKeyDown(SDLK_a) && !hitting && !IsKeyDown(SDLK_d) && !IsKeyDown(SDLK_w) && !IsKeyDown(SDLK_s) && !death) {
		if (horizontal != 2)
		{
			horizontal = 2;
			BuildSprite();
		}
		if (xpos > -0.85f)
			xpos -= deltaTime * speed;
		walk_anim = true;
	}
	if (IsKeyDown(SDLK_w) && !hitting && !IsKeyDown(SDLK_d) && !IsKeyDown(SDLK_a) && !IsKeyDown(SDLK_s) && !death) {
		if (horizontal != 4)
		{
			horizontal = 4;
			BuildSprite();
		}
		if(ypos < 0.95f)
			ypos += deltaTime * speed;
		walk_anim = true;
	}
	if (IsKeyDown(SDLK_s) && !hitting && !IsKeyDown(SDLK_d) && !IsKeyDown(SDLK_w) && !IsKeyDown(SDLK_a) && !death) {
		if (horizontal != 3)
		{
			horizontal = 3;
			BuildSprite();
		}
		if(ypos > -0.8f)
			ypos -= deltaTime * speed;
		walk_anim = true;
	}
	if (horizontal == 3)
	{
		if (!IsKeyDown(SDLK_s))
		{
			BuildSprite();
		}
	}
	else if (horizontal == 1) {
		if (!IsKeyDown(SDLK_d))
		{
			BuildSprite();
		}
	}
	else if (horizontal == 2)
	{
		if (!IsKeyDown(SDLK_a))
		{
			BuildSprite();
		}
	}
	else if (horizontal == 4)
	{
		if (!IsKeyDown(SDLK_w))
		{
			BuildSprite();
		}

	}

	mat4 transform;
	transform = translate(transform, vec3(xpos, ypos, 0.0f));

	GLint location2 = glGetUniformLocation(this->program, "transform");
	UseShader(this->program);
	glUniformMatrix4fv(location2, 1, GL_FALSE, value_ptr(transform));

	if (!hitting) {
		xspos = -2.0f;
		yspos = -2.0f;
	}
	
}

void Demo::CheckCollide() {
	for (int i = 0; i < 4; i++) {
		if (IsCollided(getPPosx(), getPPosy(), Pwidth, Pheight, enemy[i].getEPosx(), enemy[i].getEPosy(), enemy[i].getEwidth(), enemy[i].getEheight())) {
			enemy[i].Init(2.0f, 2.0f);
			enemy[i].setStatus(true);

			horizontal = 9;
			BuildSprite();
			death = true;
			
			cout << "you're dead!!" << endl;

		}
		if (IsCollided(getSPosx() - 0.075f, getSPosy() - 0.075f, Swidth, Sheight, enemy[i].getEPosx(), enemy[i].getEPosy(), enemy[i].getEwidth(), enemy[i].getEheight())) {
			enemy[i].Init(2.0f, 2.0f);
			enemy[i].setStatus(true);
			ScoreCalc();
			cout << "slayed!!" << endl;

		}

	}

}

void Demo::LoadImage(char* loadedImage) {
	unsigned char* image = SOIL_load_image(loadedImage, &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Demo::DrawSprite() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Bind Textures using texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	// Activate shader
	UseShader(this->program);

	// Draw sprite
	glBindVertexArray(VAO);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
	glDisable(GL_BLEND);
}

void Demo::BuildSprite()
{
	this->program = BuildShader("resource/spriteAnim.vert", "resource/spriteAnim.frag");
	// Pass n to shader
	GLint location = glGetUniformLocation(this->program, "n");
	UseShader(this->program);

	// Load, create texture
	if (horizontal == 5 || horizontal == 6 || horizontal == 7 || horizontal == 8 || horizontal == 9) {
		glUniform1f(location, 1.0f / NUM_FRAMES2);

		// Load and create a texture 
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

											   // Set texture filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		if (horizontal == 5) {
			LoadImage("resource/slash_depan.png");
		}
		else if (horizontal == 6)
		{
			LoadImage("resource/slash_belakang.png");
		}
		else if (horizontal == 7)
		{
			LoadImage("resource/slash_kanan.png");
		}
		else if (horizontal == 8)
		{
			LoadImage("resource/slash_kiri.png");
		}
		else if (horizontal == 9)
		{
			LoadImage("resource/dead.png");
		}
		frame_width = ((float)width) / NUM_FRAMES2;
	}
	else {
		glUniform1f(location, 1.0f / NUM_FRAMES);

		// Load and create a texture 
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

											   // Set texture filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if (horizontal == 1)
		{
			LoadImage("resource/kanan.png");
		}
		else if (horizontal == 4)
		{
			LoadImage("resource/belakang.png");
		}
		else if (horizontal == 2)
		{
			LoadImage("resource/kiri.png");
		}
		else if (horizontal == 3)
		{
			LoadImage("resource/depan.png");
		}

		frame_width = ((float)width) / NUM_FRAMES;
	}


	// Unbind texture when done, so we won't accidentily mess up our texture.

	// Set up vertex data (and buffer(s)) and attribute pointers

	float w = 0.1f;
	float x = frame_width * w / height;
	float y = w;
	Pwidth = x;
	Pheight = y;
	GLfloat vertices[] = {
		// Positions   // Colors           // Texture Coords
		x,  y, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f, // Top Right
		x, -y, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f, // Bottom Right
		-x, -y, 0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f, // Bottom Left
		-x,  y, 0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f  // Top Left 
	};

	GLuint indices[] = {  // Note that we start from 0!
		0, 3, 2, 1
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	// Color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// TexCoord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO

}

void Demo::DrawSword() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Bind Textures using texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, swordtexture);

	// Activate shader
	UseShader(this->swordprogram);

	// Draw sprite
	glBindVertexArray(swordVAO);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
	glDisable(GL_BLEND);
}

void Demo::BuildSword() {
	this->swordprogram = BuildShader("resource/swordAnim.vert", "resource/swordAnim.frag");
	// Pass n to shader
	GLint location = glGetUniformLocation(this->swordprogram, "n");
	UseShader(this->swordprogram);

	// Load, create texture
	glUniform1f(location, 1.0f / NUM_FRAMES2);

	// Load and create a texture 
	glGenTextures(1, &swordtexture);
	glBindTexture(GL_TEXTURE_2D, swordtexture); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

												// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (horizontal == 5) {
		LoadImage("resource/sword_bawah.png");
	}
	else if (horizontal == 6)
	{
		LoadImage("resource/sword_atas.png");
	}
	else if (horizontal == 7)
	{
		LoadImage("resource/sword_kanan.png");
	}
	else if (horizontal == 8)
	{
		LoadImage("resource/sword_kiri.png");
	}

	frame_width = ((float)width) / NUM_FRAMES2;



	// Unbind texture when done, so we won't accidentily mess up our texture.

	// Set up vertex data (and buffer(s)) and attribute pointers

	float w = 0.25f;
	float x = frame_width * w / height;
	float y = w;
	Swidth = x;
	Sheight = y;
	GLfloat vertices[] = {
		// Positions   // Colors           // Texture Coords
		x,  y, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f, // Top Right
		x, -y, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f, // Bottom Right
		-x, -y, 0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f, // Bottom Left
		-x,  y, 0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 1.0f  // Top Left 
	};

	GLuint indices[] = {  // Note that we start from 0!
		0, 3, 2, 1
	};

	glGenVertexArrays(1, &swordVAO);
	glGenBuffers(1, &swordVBO);
	glGenBuffers(1, &swordEBO);

	glBindVertexArray(swordVAO);

	glBindBuffer(GL_ARRAY_BUFFER, swordVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, swordEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	// Color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// TexCoord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO

	mat4 transform;
	transform = translate(transform, vec3(xspos, yspos, 0.0f));

	GLint location2 = glGetUniformLocation(this->swordprogram, "transform");
	UseShader(this->swordprogram);
	glUniformMatrix4fv(location2, 1, GL_FALSE, value_ptr(transform));
}

void Demo::DrawBG() {
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bgTex);
	GLint location2 = glGetUniformLocation(this->bgProgram, "ourTexture");
	// Activate shader
	UseShader(this->bgProgram);
	glUniform1i(location2, 1);

	// Draw crate
	glBindVertexArray(bgVAO);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
	glDisable(GL_BLEND);

}

void Demo::BuildBG() {
	this->bgProgram = BuildShader("resource/bg.vert", "resource/bg.frag");

	// Load and create a texture 
	glGenTextures(1, &bgTex);
	glBindTexture(GL_TEXTURE_2D, bgTex); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

										 // Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	LoadImage("resource/bg.png");
									 // Set up vertex data (and buffer(s)) and attribute pointers

	float w = 0.9f;
	float x = width * w / height;
	float y = w;

	GLfloat vertices[] = {
		// Positions   // Colors           // Texture Coords
		x,  y, 0.0f,   1.0f, 1.0f, 1.0f,   4.0f, 4.0f, // Top Right
		x, -y, 0.0f,   1.0f, 1.0f, 1.0f,   4.0f, 0.0f, // Bottom Right
		-x, -y, 0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f, // Bottom Left
		-x,  y, 0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 4.0f  // Top Left 
	};

	GLuint indices[] = {  // Note that we start from 0!
		0, 3, 2, 1
	};

	glGenVertexArrays(1, &bgVAO);
	glGenBuffers(1, &bgVBO);
	glGenBuffers(1, &bgEBO);

	glBindVertexArray(bgVAO);

	glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	// Color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// TexCoord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO

	mat4 transform;
	transform = translate(transform, vec3(0.0f, 0.0f, 0.0f));
	GLint location = glGetUniformLocation(this->bgProgram, "transform");
	UseShader(this->bgProgram);
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(transform));

}

void Demo::DrawWall() {
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, wallTex);
	GLint location2 = glGetUniformLocation(this->wallProgram, "ourTexture");
	// Activate shader
	UseShader(this->wallProgram);
	glUniform1i(location2, 1);

	// Draw crate
	glBindVertexArray(wallVAO);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
	glDisable(GL_BLEND);

}

void Demo::BuildWall() {
	this->wallProgram = BuildShader("resource/wall.vert", "resource/wall.frag");

	// Load and create a texture 
	glGenTextures(1, &wallTex);
	glBindTexture(GL_TEXTURE_2D, wallTex); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

										 // Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	LoadImage("resource/wall.png");
									 // Set up vertex data (and buffer(s)) and attribute pointers

	float w = 1.0f;
	float x = width * w / height;
	float y = w;

	GLfloat vertices[] = {
		// Positions   // Colors           // Texture Coords
		x,  y, 0.0f,   1.0f, 1.0f, 1.0f,   16.0f, 12.0f, // Top Right
		x, -y, 0.0f,   1.0f, 1.0f, 1.0f,   16.0f, 0.0f, // Bottom Right
		-x, -y, 0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f, // Bottom Left
		-x,  y, 0.0f,  1.0f, 1.0f, 1.0f,   0.0f, 12.0f  // Top Left 
	};
	

	GLuint indices[] = {  // Note that we start from 0!
		0, 3, 2, 1
	};

	glGenVertexArrays(1, &wallVAO);
	glGenBuffers(1, &wallVBO);
	glGenBuffers(1, &wallEBO);

	glBindVertexArray(wallVAO);

	glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	// Color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// TexCoord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO

	mat4 transform;
	transform = translate(transform, vec3(0.0f, 0.0f, 0.0f));
	GLint location = glGetUniformLocation(this->wallProgram, "transform");
	UseShader(this->wallProgram);
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(transform));

}

bool Demo::IsCollided(float x1, float y1, float width1, float height1,
	float x2, float y2, float width2, float height2) {
	return (x1 < (x2 + width2) && (x1 + width1) > x2 && y1 < (y2 + height2) && (y1 + height1) > y2);
}

void Demo::joyStickPress(int key) {
	if (key == SDL_CONTROLLER_BUTTON_DPAD_UP) {
		cout << "atas atas" << endl;
		atas = true;

	}
	else if (key == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
		cout << "bawah bawah" << endl;
		bawah = true;

	}
	else if (key == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
		cout << "kiri kiri" << endl;
		kiri = true;

	}
	else if (key == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
		cout << "kanan kanan" << endl;
		kanan = true;

	}

}

void Demo::joyStickRelease(int key) {
	if (key == SDL_CONTROLLER_BUTTON_DPAD_UP) {
		atas = false;

	}
	else if (key == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
		bawah = false;

	}
	else if (key == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
		kiri = false;

	}
	else if (key == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
		kanan = false;

	}

}




int main(int argc, char** argv) {

	Engine::Game &game = Demo();

	game.Start("Moving Boss", 800, 600, true, WindowFlag::WINDOWED, 0, 1);

	return 0;
}
