#ifndef DEMO_H
#define DEMO_H

#include <SOIL\SOIL.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtx\vector_angle.hpp>
#include <SDL\SDL_gamecontroller.h>
#include <iostream>
#include <ft2build.h>
#include <freetype\freetype.h>
#include <map>
#include <irrKlang.h>
#include <Windows.h>

#include "Game.h"

#define NUM_FRAMES 9
#define NUM_FRAMES2 6
#define FRAME_DUR 80
#define FONTSIZE 20
#define FONTNAME "fonts/ArcadeClassic.ttf"

using namespace glm;
using namespace std;
using namespace irrklang;

class Demo :
	public Engine::Game
{
	struct Character
	{
		GLuint TextureID;
		ivec2 Size;
		ivec2 Bearing;
		GLuint Advance;

	};

public:
	Demo();
	~Demo();
	virtual void Init();
	virtual void Update(float deltaTime);
	virtual void Render();
	virtual void joyStickPress(int key);
	virtual void joyStickRelease(int key);
	void InitFont();
	float getPPosx() { return xpos; };
	float getPPosy() { return ypos; };
	float getSPosx() { return xspos; };
	float getSPosy() { return yspos; };

private:
	void RenderText(string text, GLfloat x, GLfloat y, GLfloat scale, vec3 color);
	map<GLchar, Character> Characters;
	GLuint VBO, VAO, EBO, texture, program;
	GLuint bgVBO, bgVAO, bgEBO, bgTex, bgProgram;
	GLuint wallVBO, wallVAO, wallEBO, wallTex, wallProgram;
	GLuint enemyVBO, enemyVAO, enemyEBO, enemyTex, enemyProgram;
	GLuint swordVBO, swordVAO, swordEBO, swordtexture, swordprogram;
	GLuint fontVBO, fontVAO, fontProgram;
	float frame_dur = 0, frame_width = 0, xpos = 0, ypos = 0, xspos = -2, yspos = -2, Pwidth, Pheight, Swidth, Sheight;
	float enemFrame_dur = 0, enemFrame_width = 0;
	const char* MP3NAME = "resource/Band of Thieves.mp3";
	unsigned int frame_idx = 0;
	unsigned int frame_idx2 = 0;
	unsigned int enem_idx = 0, eFlip = 0;

	void BuildSprite();
	void BuildSword();
	void BuildBG();
	void BuildWall();
	void DrawSprite();
	void DrawSword();
	void DrawBG();
	void DrawWall();

	void InitAndPlayMusic();
	void UpdateSprite(float deltaTime);
	void MoveSprite(float deltaTime);
	bool IsCollided(float x1, float y1, float width1, float height1, float x2, float y2, float width2, float height2);
	void CheckCollide();
	void EnemySpawn(float deltaTime);
	void ScoreCalc();
	void LoadImage(char* loadedImage);
	void QuitHandler(float deltaTime);
	
	bool walk_anim = false;
	bool vertical = false;
	bool slash = false;
	bool hitting = false;
	bool walking = false;
	bool prio = false;
	bool death = false;
	int intSlash = 0;
	int horizontal = 1;
	int width, height;
	int spawnDelay = 5000;
	float countDown = 0;
	float spawnDelta = 0;
	int score = 0;
	int enemySpawned = 0;
	float speed = 0.0005f;
	bool atas = false, bawah = false, kanan = false, kiri = false;

};
#endif

