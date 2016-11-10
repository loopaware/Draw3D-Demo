/*
 * main.cpp
 *
 *  Created on: 16 okt. 2016
 *      Author: bladd
 */

#include <iostream> // Debugging purposes
#include <string>
#include <vector>

// OpenGL / glew Headers
#define GL3_PROTOTYPES 1
#include <GL/glew.h>

// SDL2 Headers
#include <SDL2/SDL.h>
//#include <SDL2/SDL_opengl.h> // substitute for gl.h?
//#include <SDL2/SDL_image.h>
#include <glm/glm.hpp>

#include "Functions.h"
#include "classes/ShaderProgram.h" // move this somewhere else

using namespace std;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

string 		appName 				= "Draw3D - Prototype";
const int 	SCREEN_FPS 				= 60;
const int 	SCREEN_TICKS_PER_FRAME 	= 1000/SCREEN_FPS;

SDL_Window          *wndMain;		// SDL window handle
SDL_Renderer        *sdlRenderer;	// SDL renderer handle
SDL_RendererInfo    rendererInfo;	// SDL renderer info
SDL_GLContext       mainContext;	// OGL rendering context

float fpsDeltaTime;	// Delta time in seconds
float fpsClock;		// Last time sample in seconds
float fpsRenderTimer;	// Time constrol for rendering

ShaderProgram* shader;

struct Vertex
{
	Vertex(float x, float y, float z):
		position(x, y, z)
	{}

	vec3 position;
};

GLuint vao;
GLuint vbo;
vector<Vertex> vertices;

bool init();
	bool initSDLGL();
		const string popSDLError(const int& line);
		void setGLAttributes();
		const string getRendererInfoText();
	void importMesh();
bool appLoop();
	void update();
		void handleEvents(bool& loop);
		void handlePicking();
	void render();
bool cleanup();

int main(int argc, char *argv[])
{
	std::cout << "Demo application launched." << std::endl;

	cout << "Initializing..." << endl;
	if (!init())
	{
		cout << "Error encountered." << endl;;
		cout << popSDLError(__LINE__) << endl;
		return 1;
	}

	cout << "Entering loop..." << endl;
	if (!appLoop())
	{
		cout << "Error encountered." << endl;;
		cout << popSDLError(__LINE__) << endl;
		return 1;
	}

	cout << "Cleaning up..." << endl;
	if (!cleanup())
	{
		cout << "Error encountered." << endl;;
		cout << popSDLError(__LINE__) << endl;
		return 1;
	}

	std::cout << "Demo application closed." << std::endl;

	return 0;
}

bool init()
{
	//init(argc, argv);
	wndMain 	= nullptr;  // SDL window handle
	sdlRenderer	= nullptr;	// SDL renderer handle

	fpsDeltaTime 	= 0.0f;
	fpsClock 		= SDL_GetTicks();
	fpsRenderTimer 	= 0.0f;


	if (!initSDLGL())
	{
		return false;;
	}

	return true;
}

bool initSDLGL()
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		std::cout << "Oops! SDL2 init error." << std::endl;
		cout << popSDLError(__LINE__);
		return false;
	}

	// Create the window where we will draw.
	wndMain = SDL_CreateWindow("draw3d - prototype",                             	// Window title
							  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,   //
							  512, 512,                        					// Window size
							  SDL_WINDOW_OPENGL);                                //

	// Check for errors
	if (!wndMain)
	{
		cout << "Unable to create window." << endl;
		return false;
	}

	mainContext = SDL_GL_CreateContext(wndMain);

	setGLAttributes();

	// vsync
	SDL_GL_SetSwapInterval(1);

	// Init GLEW
	// Apparently, this is needed for Apple. Thanks to Ross Vander for letting me know
	#ifndef __APPLE__
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		// Problem: glewInit failed, something is seriously wrong.
		std::cout << "Glew error: " << glewGetErrorString(err) << std::endl;
		return false;
	}
	#endif

	// Shader
	shader = new ShaderProgram();
	shader->addShader("shaders/default-vertex.glsl", GL_VERTEX_SHADER);
	shader->addShader("shaders/default-fragment.glsl", GL_FRAGMENT_SHADER);
	shader->compile();
	shader->link();

	return true;
}

const string popSDLError(const int& line)
{
	std::string error = SDL_GetError();

	if (error != "")
	{
		std::cout << "SLD Error : " << error << std::endl;

		if (line != -1)
			std::cout << "\nLine : " << line << std::endl;

		SDL_ClearError();

		string tmp = "";
		tmp += "SDL Error: ";
		tmp += error;
		tmp += "\nLine: ";
		tmp += line;
		tmp += "\n";

		return tmp;
	}

	return "";
}

void setGLAttributes()
{
	// Set our OpenGL version.
	// SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions are disabled
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// 3.2 is part of the modern versions of OpenGL, but most video cards whould be able to run it
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	// Turn on double buffering with a 24bit Z buffer.
	// You may need to change this to 16 or 32 for your system
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	//	// Enable smooth shading
		//	    glShadeModel( GL_SMOOTH );
		//
		//	    // Set the background black
		//	    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
		//
		//	    // Depth buffer setup
		//	    glClearDepth( 1.0f );
		//
		//	    // Enables Depth Testing
		//	    glEnable( GL_DEPTH_TEST );
		//
		//	    // The Type Of Depth Test To Do
		//	    glDepthFunc( GL_LEQUAL );
		//
		//	    // Really Nice Perspective Calculations
		//	    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
}

const string getRendererInfoText()
{
	return "gg";
}

void importMesh()
{
	// Generate triangle
	vertices.push_back(Vertex(-0.5f, -0.5f, 0.0f));
	vertices.push_back(Vertex(0.5f, -0.5f, 0.0f));
	vertices.push_back(Vertex(0.0f, 0.5f, 0.0f));

	// Send to gpu
	vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	GLuint positionID = 0;
	glEnableVertexAttribArray(positionID);
	glVertexAttribFormat(0, 3, GL_FLOAT, false, 0);
	glVertexAttribBinding(0, 0);
	vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));

	glBindVertexArray(0);
}

bool appLoop()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapWindow(wndMain);

	importMesh();

	bool loop = true;
	while (loop)
	{
		fpsDeltaTime = SDL_GetTicks() - fpsClock;	// Get the current delta time for this frame
		fpsClock = SDL_GetTicks();					// Updates the clock to check the next delta time

		handleEvents(loop);
		update();

		if (fpsRenderTimer >= (1.0f/60.0f))
		{
			render();
			fpsRenderTimer -= (1.0f/60.0f);	 //do not set to zero, remove the accumulated frame time to avoid skipping
		}

		fpsRenderTimer += fpsDeltaTime; // Updates the render timer
	}

	return true;
}

void update()
{

}

void handleEvents(bool& loop)
{
	SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT)
					loop = false;

				if (event.type == SDL_KEYDOWN)
				{
					switch (event.key.keysym.sym)
					{
					case SDLK_ESCAPE:
						loop = false;
						break;
					case SDLK_r:
						// Cover with red and update
						glClearColor(1.0, 0.0, 0.0, 1.0);
						glClear(GL_COLOR_BUFFER_BIT);
						break;
					case SDLK_g:
						// Cover with green and update
						glClearColor(0.0, 1.0, 0.0, 1.0);
						glClear(GL_COLOR_BUFFER_BIT);
						break;
					case SDLK_b:
						// Cover with blue and update
						glClearColor(0.0, 0.0, 1.0, 1.0);
						glClear(GL_COLOR_BUFFER_BIT);
						break;

					// mousebtn
						// handlePicking();

					default:
						break;
					}
				}
			}

}

void handlePicking();

void render()
{
	glClear(GL_COLOR_BUFFER_BIT);

	shader->bindShader();

	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glDisableVertexAttribArray(0);

	shader->unbindShader();

	SDL_GL_SwapWindow(wndMain);
	//SDL_RenderClear(sdl_renderer);
	//SDL_GL_SwapWindow(sdl_window);
}

bool cleanup()
{
	// Delete shaders
	delete shader;

	// Once finished with OpenGL functions, the SDL_GLContext can be deleted.
	SDL_GL_DeleteContext(mainContext);

	//Destroy window
	SDL_DestroyWindow(wndMain);

	//Quit SDL subsystems
	SDL_Quit();

	return true;
}
