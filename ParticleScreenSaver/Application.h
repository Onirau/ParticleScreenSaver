#ifndef Application_H
#define Application_H

#define _USE_MATH_DEFINES

#include <math.h>
#include <iostream>
#include <string>
#include <vector>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Vector2.h"

using namespace std;

enum class ShaderType {
	Vertex = GL_VERTEX_SHADER,
	Fragment = GL_FRAGMENT_SHADER,
	Compute = GL_COMPUTE_SHADER
};

struct Shader {
	const char* name = nullptr;
	ShaderType type;
	const char* source;
};

struct Particle {
	float position[2];
	float velocity[2];
	float age; // current
	float life; // max
};

struct AttributeLocation {
	GLuint location;
	GLint num_components;
	GLsizei stride;
	GLenum type;
};

struct UpdateAttributeLocations {
	AttributeLocation i_Position;
	AttributeLocation i_Age;
	AttributeLocation i_Life;
	AttributeLocation i_Velocity;
};

struct RenderAttributeLocations {
	AttributeLocation i_Position;
};

class Application {
private:
	double _applicationStartTime;
	double _applicationCurrentTime;
	double _applicationLastUpdate;
	double _applicationLastDisplayUpdate;
	double _applicationFrameCount;

	GLFWwindow* _window;

	// Particles
	GLuint _particleBuffers[2]; // Buffers
	GLuint _particleVAO[4];
	int _read = 0;
	int _write = 1;

	GLuint _updateProgram, _renderProgram; // Programs
	
	GLuint _noiseTexture;

	void _update(double tt, double dt);
	static void _key_callback(GLFWwindow window, int key, int scancode, int action, int mods);
public:
	const char* title;
	int numParticles;
	float minAge, maxAge;
	float gravity[2] = { 0.0, -0.8 };
	float origin[2] = { 0.0, 0.0 };
	float theta[2] = { M_PI / 2.0 - 0.5, M_PI / 2.0 + 0.5 };
	float speed[2] = { 0.5, 1.0f };
	IntVector2 windowDimensions;

	Application(const char* title, int _numParticles, float minAge, float maxAge, IntVector2 _windowDimensions);
	void run();
	void createWindow();
	void compileShaders();
	void setupBuffers();
	void genBuffers();
};

#endif // !Application_H
