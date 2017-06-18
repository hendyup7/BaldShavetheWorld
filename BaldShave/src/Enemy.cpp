#include "Enemy.h"

Enemy::Enemy() {

}

Enemy::~Enemy() {

}

void Enemy::Init(float x, float y) {
	BuildEnemy(x, y);

}

void Enemy::Init() {
	BuildEnemy(0.0f, 0.0f);

}

void Enemy::Update(float deltaTime, float Pposx, float Pposy) {
	UpdateEnemy(deltaTime);
	FollowPlayer(Pposx, Pposy);
	MoveEnemy(deltaTime);

}

void Enemy::Render() {
	if (!dead)
		DrawEnemy();

}

void Enemy::UpdateEnemy(float deltaTime) {
	enemFrame_dur += deltaTime;

	if (enemFrame_dur > FRAME_DUR) {
		enemFrame_dur = 0;
		if (enem_idx == ENEMY_FRAMES - 1) enem_idx = 0; else enem_idx++;

		GLint location = glGetUniformLocation(this->enemyProgram, "frameIndex");
		UseShader(this->enemyProgram);
		glUniform1i(location, enem_idx);

	}

}

void Enemy::FollowPlayer(float Pposx, float Pposy) {
	if (Eposx > Pposx) {
		eKanan = false;
		eKiri = true;

	}
	else if (Eposx < Pposx) {
		eKiri = false;
		eKanan = true;

	}
	else {
		eKiri = eKanan = false;

	}
	if (Eposy > Pposy) {
		eBawah = true;
		eAtas = false;

	}
	else if (Eposy < Pposy) {
		eAtas = true;
		eBawah = false;

	}
	else {
		eAtas = eBawah = false;

	}

}

void Enemy::MoveEnemy(float deltaTime) {
	if (eAtas)
		Eposy += deltaTime * eSpeed;
	if (eBawah)
		Eposy -= deltaTime * eSpeed;
	if (eKanan) {
		Eposx += deltaTime * eSpeed;
		eFlip = 1;
	}
	if (eKiri) {
		Eposx -= deltaTime * eSpeed;
		eFlip = 0;
	}

	mat4 transform;
	transform = translate(transform, vec3(Eposx, Eposy, 0.0f));
	GLint location = glGetUniformLocation(this->enemyProgram, "transform");
	GLint location2 = glGetUniformLocation(this->enemyProgram, "flip");
	UseShader(this->enemyProgram);
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(transform));
	glUniform1i(location2, eFlip);

}

void Enemy::DrawEnemy() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Bind Textures using texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, enemyTex);
	//GLint location = glGetUniformLocation(this->enemyProgram, "ourTexture");
	// Activate shader
	UseShader(this->enemyProgram);
	//glUniform1i(location, 0);

	// Draw sprite
	glBindVertexArray(enemyVAO);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

}

void Enemy::BuildEnemy(float startx, float starty) {
	this->enemyProgram = BuildShader("resource/enemy.vert", "resource/enemy.frag");

	// Pass n to shader
	GLint location = glGetUniformLocation(this->enemyProgram, "n");
	UseShader(this->enemyProgram);
	glUniform1f(location, 1.0f / ENEMY_FRAMES);

	// Load and create a texture 
	glGenTextures(1, &enemyTex);
	glBindTexture(GL_TEXTURE_2D, enemyTex); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

											// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load, create texture 
	int width, height;
	unsigned char* image = SOIL_load_image("resource/enemy.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.

									 // Set up vertex data (and buffer(s)) and attribute pointers
	enemFrame_width = ((float)width) / ENEMY_FRAMES;
	float w = 0.05f;
	float x = enemFrame_width * w / height;
	float y = w;
	Ewidth = x * 2;
	Eheight = y * 2;
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

	glGenVertexArrays(1, &enemyVAO);
	glGenBuffers(1, &enemyVBO);
	glGenBuffers(1, &enemyEBO);

	glBindVertexArray(enemyVAO);

	glBindBuffer(GL_ARRAY_BUFFER, enemyVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, enemyEBO);
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

	Eposx = startx;
	Eposy = starty;

}
