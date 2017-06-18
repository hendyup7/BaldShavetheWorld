#pragma once
#ifndef ENEMY_H
#define ENEMY_H

#include <SOIL\SOIL.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "Demo.h"

#define ENEMY_FRAMES 4
#define FRAME_DUR 80

using namespace std;
using namespace glm;

class Enemy : 
	public Demo{

public:
	Enemy();
	~Enemy();
	void Init(float x, float y);
	void Init();
	void Update(float deltaTime, float Pposx, float Pposy);
	void Render();
	float getEPosx() { return Eposx; };
	float getEPosy() { return Eposy; };
	float getEwidth() { return Ewidth; };
	float getEheight() { return Eheight; };
	void setStatus(bool status) { dead = status; };
	bool getStatus() { return dead; };
	void setSpeed(float speed) { eSpeed = speed; };

private:
	GLuint enemyVBO, enemyVAO, enemyEBO, enemyTex, enemyProgram;
	float enemFrame_dur = 0, enemFrame_width = 0;
	unsigned int enem_idx = 0, eFlip = 0;
	void BuildEnemy(float startx, float starty);
	void DrawEnemy();
	void UpdateEnemy(float deltaTime);
	void MoveEnemy(float deltaTime);
	void FollowPlayer(float Pposx, float Pposy);

	float Ewidth, Eheight;
	bool dead = true;
	float eSpeed = 0.0001f;
	float Eposx, Eposy;
	bool eAtas, eBawah, eKanan, eKiri;

};

#endif // !ENEMY_H
