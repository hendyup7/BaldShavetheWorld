#ifndef GAME_H
#define GAME_H

#include <SDL\SDL.h>
#include <GL\glew.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <SDL\SDL_gamecontroller.h>
#include <unordered_map>

using namespace std;

enum class State { RUNNING, EXIT };
enum class WindowFlag { WINDOWED, FULLSCREEN, EXCLUSIVE_FULLSCREEN, BORDERLESS};

namespace Engine {
	class Game
	{
	public:
		Game();
		~Game();
		void Start(string title, unsigned int width, unsigned int height, bool vsync, WindowFlag windowFlag, unsigned int targetFrameRate, float timeScale);
		// Input Handling
		void PressKey(unsigned int keyID);
		void ReleaseKey(unsigned int keyID);
		/// Returns true if the key is held down
		bool IsKeyDown(unsigned int keyID);
		bool IsKeyPressed(unsigned int keyID);

		float RandomNumber(float kecil, float besar);

		void setState(State sta) { state = sta; };
	protected:
		virtual void Init() = 0;
		virtual void Update(float deltaTime) = 0;
		virtual void Render() = 0;
		virtual void joyStickPress(int key) = 0;
		virtual void joyStickRelease(int key) = 0;
		GLuint BuildShader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
		void UseShader(GLuint program);
		unsigned int screenWidth, screenHeight;
	private:
		unsigned int lastFrame = 0, last = 0, _fps = 0, fps = 0;
		float targetFrameTime = 0, timeScale;
		State state;
		float GetDeltaTime();
		void GetFPS();
		void PollInput();
		void Err(string errorString);
		void LimitFPS();
		void CheckShaderErrors(GLuint shader, string type);

		SDL_GameController *gameController = NULL;
		void initJoystick();

		// Input Handling
		/// Returns true if the key is held down
		bool WasKeyDown(unsigned int keyID);
		unordered_map<unsigned int, bool> _keyMap;
		unordered_map<unsigned int, bool> _previousKeyMap;

	};
}
#endif

