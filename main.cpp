/*
 * main.cpp
 *
 *  Created on: 16 okt. 2016
 *      Author: bladd
 */

//#pragma pack(1) // Align data as seen in structs

#include <iostream> // Debugging purposes
#include <string>
#include <vector>

// OpenGL / glew Headers
#define GL3_PROTOTYPES 1
#include <GL/glew.h>

// SDL2 Headers
#include <SDL2/SDL.h>
//#include <SDL2/SDL_opengl.h> // substitute for gl.h?
#include <SDL2/SDL_image.h>

//#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <fstream>

//#include "Functions.h"
#include "classes/ShaderProgram.h" // move this somewhere else

using namespace std;
using glm::ivec2;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using glm::rotate;
using glm::scale;
using glm::inverse;
using glm::transpose;

string 		appName 				= "Draw3D - Prototype";
const int 	SCREEN_FPS 				= 60;
const int 	SCREEN_TICKS_PER_FRAME 	= 1000/SCREEN_FPS;

SDL_Window          *wndMain;		// SDL window handle
SDL_Renderer        *sdlRenderer;	// SDL renderer handle
SDL_RendererInfo    rendererInfo;	// SDL renderer info
SDL_GLContext       mainContext;	// OGL rendering context

float fpsDeltaTime;		// Delta time in seconds
float fpsClock;			// Last time sample in seconds
float fpsRenderTimer;	// Time control for rendering

unsigned int renderMode; // 1 = regular, 2 = picking

ShaderProgram* shader;
ShaderProgram* shaderPicking;

struct FileHeader
{
	unsigned int		version[3];
	unsigned long long	vertexCount;
};

struct Vertex
{
	Vertex(const float x, const float y, const float z,
			const float u, const float v,
			const float nx, const float ny, const float nz):
		position(x, y, z),
		texCoords(u, v),
		normal(nx, ny, nz)
	{}

	vec3 position;
	vec2 texCoords;
	vec3 normal;
};

const unsigned int BTN_NEUTRAL 	= 0;
const unsigned int BTN_PRESSED 	= 1;
const unsigned int BTN_HELD 	= 2;
const unsigned int BTN_RELEASED = 3;

class MouseInfo
{
private:
	unsigned int 	m_LMBState;
	unsigned int	m_RMBState;
	int 			m_ScrollAccumulator;
	ivec2 			m_LocalPosition;

public:
	MouseInfo();
	MouseInfo(const MouseInfo& mouseInfo);
	~MouseInfo();

	void update();

	const unsigned int getLMBState() const;
	const unsigned int getRMBState() const;
	const ivec2 getLocalPosition() const;

	const int getScrollAccumulatorValue() const;
	void incScrollAccumulator();
	void decScrollAccumulator();
	void emptyScrollAccumulator();
};

MouseInfo::MouseInfo():
		m_LMBState(BTN_NEUTRAL),
		m_RMBState(BTN_NEUTRAL),
		m_ScrollAccumulator(0),
		m_LocalPosition(ivec2(0))
{
}

MouseInfo::MouseInfo(const MouseInfo& mouseInfo):
		m_LMBState(mouseInfo.getLMBState()),
		m_RMBState(mouseInfo.getRMBState()),
		m_ScrollAccumulator(mouseInfo.getScrollAccumulatorValue()),
		m_LocalPosition(mouseInfo.getLocalPosition())
{
}

MouseInfo::~MouseInfo()
{
}

void MouseInfo::update()
{
	// Buttons
	if (SDL_GetMouseState(nullptr, nullptr)&SDL_BUTTON(SDL_BUTTON_LEFT))
	{
		if (m_LMBState == BTN_NEUTRAL)
		{
			m_LMBState = BTN_PRESSED;
		}
		else
		{
			m_LMBState = BTN_HELD;
		}
	}
	else
	{
		if (m_LMBState == BTN_HELD)
		{
			m_LMBState = BTN_RELEASED;
		}
		else
		{
			m_LMBState = BTN_NEUTRAL;
		}
	}

	if (SDL_GetMouseState(nullptr, nullptr)&SDL_BUTTON(SDL_BUTTON_RIGHT))
		{
			if (m_RMBState == BTN_NEUTRAL)
			{
				m_RMBState = BTN_PRESSED;
			}
			else
			{
				m_RMBState = BTN_HELD;
			}
		}
		else
		{
			if (m_RMBState == BTN_HELD)
			{
				m_RMBState = BTN_RELEASED;
			}
			else
			{
				m_RMBState = BTN_NEUTRAL;
			}
		}

	// Position
	int x, y;
	x = 0;
	y = 0;
	SDL_GetMouseState(&x, &y);
	m_LocalPosition = ivec2(x, y);
}

const unsigned int MouseInfo::getLMBState() const
{
	return m_LMBState;
}

const unsigned int MouseInfo::getRMBState() const
{
	return m_RMBState;
}

const ivec2 MouseInfo::getLocalPosition() const
{
	return m_LocalPosition;
}

const int MouseInfo::getScrollAccumulatorValue() const
{
	return m_ScrollAccumulator;
}

void MouseInfo::incScrollAccumulator()
{
	m_ScrollAccumulator++;
}

void MouseInfo::decScrollAccumulator()
{
	m_ScrollAccumulator--;
}

void MouseInfo::emptyScrollAccumulator()
{
	m_ScrollAccumulator = 0;
}

MouseInfo* mouseInfo;
ivec2 lastLoc;

GLuint vao;
GLuint vbo;
vector<Vertex> vertices;

SDL_Surface* res_texture;
GLuint texDiffuse;

GLuint fbo_id;
GLuint fbo_tex;

GLuint pbo_id;

mat4 modelMatrix;
mat4 viewMatrix;
mat4 projectionMatrix;
mat3 normalMatrix;

bool init();
	bool initSDLGL();
		const string popSDLError(const int& line);
		void setGLAttributes();
		const string getRendererInfoText();
	void initCamera();
	void importMesh();
		void readMeshFile();
	void importTexture();
	void prepareFBO();
	void preparePBO();
bool appLoop();
	void update(bool &loop);
		void handleEvents(bool& loop);
		void handlePicking();
			void plotLine(const ivec2 P1, const ivec2 P2);
				const vec2 getUVFromFBO(const GLuint fboID, const ivec2 loc);
				void setPixelInTexture(GLint x, GLint y);
	void render();
		void renderMesh();
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

	res_texture = nullptr;

	renderMode = 1;

	mouseInfo = new MouseInfo();
	lastLoc = ivec2(mouseInfo->getLocalPosition());

	modelMatrix = mat4(1);
	viewMatrix = mat4(1);
	projectionMatrix = mat4(1);
	normalMatrix = mat3(1);

	initCamera();

	if (!initSDLGL())
	{
		return false;;
	}

	// Shader
	shader = new ShaderProgram();
	shader->addShader("shaders/default-vertex.glsl", GL_VERTEX_SHADER);
	shader->addShader("shaders/default-fragment.glsl", GL_FRAGMENT_SHADER);
	shader->compile();
	shader->link();

	// ShaderPicking
	shaderPicking = new ShaderProgram();
	shaderPicking->addShader("shaders/picking-vertex.glsl", GL_VERTEX_SHADER);
	shaderPicking->addShader("shaders/picking-fragment.glsl", GL_FRAGMENT_SHADER);
	shaderPicking->compile();
	shaderPicking->link();

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

	//Initialize PNG loading
	int imgFlags = IMG_INIT_PNG;
	if( !( IMG_Init( imgFlags ) & imgFlags ) )
	{
		cout << "SDL_image could not initialize! SDL_image Error." << IMG_GetError() << endl;
		cout << popSDLError(__LINE__);
		return false;
	}

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

	// Print renderer info
	cout << getRendererInfoText() << endl;

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
			    glShadeModel( GL_SMOOTH );
		//
		//	    // Set the background black
		//	    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
		//
		//	    // Depth buffer setup
			    glClearDepth( 1.0f );
		//
			    // Enables Depth Testing
		//	    glEnable( GL_DEPTH_TEST );
		//
			    // The Type Of Depth Test To Do
			    glDepthFunc( GL_LEQUAL );
		//
		//	    // Really Nice Perspective Calculations
			    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

	glEnable(GL_DEPTH_TEST); // Not on by default?? (Fixes problem with z-buffer. Occluded triangles visible thsough other triangles if drawn out of order.)

	glFrontFace(GL_CCW);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

const string getRendererInfoText()
{
	const unsigned char* cVersion = glGetString(GL_VERSION);
	cout << "Using OpenGL version: "  << cVersion << ".\n";

	// Get GPU info
	const GLubyte* renderer		= glGetString(GL_RENDERER);					// Renderer
	const GLubyte* vendor		= glGetString(GL_VENDOR);					// GPU vendor
	const GLubyte* version		= glGetString(GL_VERSION);					// OpenGL, newest supported, version
	const GLubyte* glslVersion	= glGetString(GL_SHADING_LANGUAGE_VERSION);	// Supported GLSL version
	GLint major, minor;														// Declare variables for next 2 lines...
	glGetIntegerv(GL_MAJOR_VERSION, &major);								// Get OpenGL major version
	glGetIntegerv(GL_MINOR_VERSION, &minor);								// Get OpenGL minor version

	// Print GPU info
	cout << "Running OpenGL on " << vendor << " " << renderer << ".\n";
	cout << "OpenGL version supported " << version << ".\n";
	cout << "GLSL version supported " << glslVersion << ".\n";
	cout << "Will attempt to use GL version " << major << "." << minor << ".\n";

	return "";
}

void initCamera()
{
	// Move camera, dolly (note: moving camera need an inverted vector)
	//viewMatrix = translate(viewMatrix, vec3(0, 0, -5));

	// View Matrix, lookat
	viewMatrix = glm::lookAt(
		vec3(0, 1, 2), // the position of your camera, in world space (not inverted vector here?)
		vec3(0, 0.5, 0), // where you want to look at, in world space
		vec3(0, 1, 0)  // probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
	);


	// Projection Matrix
	projectionMatrix = glm::perspective(
	    glm::radians(90.0f),         // The horizontal Field of View, in degrees : the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
	    1.0f / 1.0f, // Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar ?
	    0.1f,        // Near clipping plane. Keep as big as possible, or you'll get precision issues.
	    100.0f       // Far clipping plane. Keep as little as possible.
	);
}

void importMesh()
{
	cout << "Vertex Definition:" << endl;
	cout << "\t- SizeOf Vertex: " << sizeof(Vertex) << endl;
	cout << "\t- OffsetOf Vertex-pos: " << offsetof(Vertex, position) << endl;
	cout << "\t- OffsetOf Vertex-uv: " << offsetof(Vertex, texCoords) << endl;
	cout << "\t- OffsetOf Vertex-norm: " << offsetof(Vertex, normal) << endl;

	readMeshFile();
/*
	// Dump mesh  to console
	cout << "Current Mesh Data:" << endl;
	cout << "\t- VertexCount: " << vertices.size() << endl;
	for (vector<Vertex>::const_iterator i = vertices.begin(); i < vertices.end(); ++i)
	{
		cout << "\t- vP(" << (*i).position.x << ", " << (*i).position.y << ", " << (*i).position.z << ")"
				<< " vT(" << (*i).texCoords.x << ", " << (*i).texCoords.y << ")"
				<< " vN(" << (*i).normal.x << ", " << (*i).normal.y << ", " << (*i).normal.z << ")" << endl;
	}
*/
	// Send to gpu
	vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	const uint32_t positionID	= 0;
	const uint32_t texCoordsID	= 1;
	const uint32_t normalID		= 2;


	//glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
	//glVertexAttribBinding(0, 0);


	//glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 0);
	//glVertexAttribBinding(1, 0);

	vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texCoords)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

	glEnableVertexAttribArray(positionID);
	glEnableVertexAttribArray(texCoordsID);
	glEnableVertexAttribArray(normalID);

	glBindVertexArray(0);
}

void readMeshFile()
{
	FileHeader headerData;
	string filename = "meshes/rabbit.f3d";
	cout << "Reading f3d-file: " << filename << endl;

	ifstream file(filename, ios::in | ios::binary);
	if (!file.is_open())
	{
		cout << "Cannot read f3d-file." << endl;
		return;
	}

	// Version
	file.read((char*)&headerData.version, sizeof(unsigned int) *3);

	// Vertex Count
	file.read((char*)&headerData.vertexCount, sizeof(unsigned long long));

	// Print File Header data
	cout << "F3D Data:\n\t- Version:" << headerData.version[0] << "." << headerData.version[1] << "." << headerData.version[2] << "\n\t- Vertex Count: " << headerData.vertexCount << endl;

	// Version compatability check
	if (headerData.version[0] != 1)
	{
		cout << "Incompatible f3d-file format." << endl;
		return;
	}

	// Read vertices
	vertices.reserve(headerData.vertexCount);
	for (unsigned long long i = 0; i < headerData.vertexCount; i++)
	{
		vec3 p 	= vec3(0);
		vec2 t = vec2(0);
		vec3 n 	= vec3(0);
		file.read((char*)&p, 	sizeof(float) *3);
		file.read((char*)&t, 	sizeof(float) *2);
		file.read((char*)&n, 	sizeof(float) *3);
		vertices.push_back(Vertex(p.x, p.y, p.z, t.x, t.y, n.x, n.y, n.z));
		//cout << "\t- (" << p.x << " " << p.y << " " << p.z << ") (" << t.x << " " << t.y << ") (" << n.x << " " << n.y << " " << n.z << ")" << endl;
	}

	// Close file
	file.close();
}

void importTexture()
{
	res_texture = IMG_Load("textures/rabbit_diffuse-oil.png");
	if (res_texture == NULL) {
		cerr << "IMG_Load: " << SDL_GetError() << endl;
		return;
	}

	cout << "Texture info:" << endl;
	cout << "\t- Bits per Pixel: " << (int)res_texture->format->BitsPerPixel << endl;

	glGenTextures(1, &texDiffuse);
	glBindTexture(GL_TEXTURE_2D, texDiffuse);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, // target
		0,  // level, 0 = base, no minimap,
		GL_RGBA, // internalformat
		res_texture->w,  // width
		res_texture->h,  // height
		0,  // border, always 0 in OpenGL ES
		GL_RGBA,  // format
		GL_UNSIGNED_BYTE, // type
		res_texture->pixels);

	// Free memory
	SDL_FreeSurface(res_texture);
}

void prepareFBO()
{
	glGenFramebuffers(1, &fbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

	// Color
	glGenTextures(1, &fbo_tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fbo_tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_tex, 0);

	// Set the target for the fragment shader outputs
	GLenum drawBufs[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawBufs);

	// Depth
	GLuint depthBuf;
	glGenRenderbuffers(1, &depthBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 512, 512);

	// Bind the depth buffer to the FBO
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);


	GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if( result == GL_FRAMEBUFFER_COMPLETE) {
		cout << "Frame buffer is complete" << endl;
	} else {
		cout << "Frame buffer error: " << result << endl;
	}

	// Unbind the framebuffer, and revert to default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void preparePBO()
{
	glGenBuffers(1, &pbo_id);

}

void bunchOfCode()
{
	/*
	// create 2 pixel buffer objects, you need to delete them when program exits.
	        // glBufferDataARB with NULL pointer reserves only memory space.
	        glGenBuffersARB(PBO_COUNT, pboIds);
	        glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[0]);
	        glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, DATA_SIZE, 0, GL_STREAM_READ_ARB);
	        glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[1]);
	        glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, DATA_SIZE, 0, GL_STREAM_READ_ARB);

	        glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);

*/
}

bool appLoop()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	SDL_GL_SwapWindow(wndMain);

	importMesh();
	importTexture();
	prepareFBO();
	//preparePBO();

	bool loop = true;
	while (loop)
	{
		fpsDeltaTime = SDL_GetTicks() - fpsClock;	// Get the current delta time for this frame
		fpsClock = SDL_GetTicks();					// Updates the clock to check the next delta time

		update(loop);

		if (fpsRenderTimer >= (1.0f/60.0f))
		{
			render();
			fpsRenderTimer -= (1.0f/60.0f);	 //do not set to zero, remove the accumulated frame time to avoid skipping
		}

		fpsRenderTimer += fpsDeltaTime; // Updates the render timer
	}

	return true;
}

void update(bool &loop)
{
	handleEvents(loop);
	lastLoc = mouseInfo->getLocalPosition();
	mouseInfo->update();
	handlePicking();

	// Handle scroll scale
	if (mouseInfo->getScrollAccumulatorValue() != 0)
	{
		modelMatrix = glm::scale(modelMatrix, vec3(mouseInfo->getScrollAccumulatorValue()*0.1 +1));
		//cout << "AccVal: " << mouseInfo->getScrollAccumulatorValue() << endl;
		mouseInfo->emptyScrollAccumulator();
	}

	// Handle
	if (mouseInfo->getRMBState() == BTN_HELD)
	{
		ivec2 mDelta = mouseInfo->getLocalPosition() - lastLoc;
		modelMatrix = glm::rotate(modelMatrix, (float)mDelta.x/100, vec3(0, 1, 0));
		modelMatrix = glm::rotate(modelMatrix, (float)mDelta.y/100, vec3(1, 0, 0));
	}

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
					case SDLK_1:
						// Set render mode to regular
						renderMode = 1;
						break;
					case SDLK_2:
						// Set rendermode to picking
						renderMode = 2;
						break;

					case SDLK_a:
						// Rotate around Y
						modelMatrix = rotate(modelMatrix, 0.2f, vec3(0, 1, 0));
						break;
					case SDLK_d:
						// Rotate around Y
						modelMatrix = rotate(modelMatrix, -0.2f, vec3(0, 1, 0));;
						break;
					case SDLK_w:
						// Rotate around X
						modelMatrix = rotate(modelMatrix, 0.2f, vec3(1, 0, 0));
						break;
					case SDLK_s:
						// Rotate around X
						modelMatrix = rotate(modelMatrix, -0.2f, vec3(1, 0, 0));;
						break;
					case SDLK_q:
						// Rotate around Z
						modelMatrix = rotate(modelMatrix, 0.2f, vec3(0, 0, 1));
						break;
					case SDLK_e:
						// Rotate around Z
						modelMatrix = rotate(modelMatrix, -0.2f, vec3(0, 0, 1));;
						break;

					case SDLK_PLUS:
						// Rotate around Z
						modelMatrix = scale(modelMatrix, vec3(1.01f, 1.01f, 1.01f));
						break;
					case SDLK_MINUS:
						// Rotate around Z
						modelMatrix = scale(modelMatrix, vec3(0.99f, 0.99f, 0.99f));;
						break;

					default:
						break;
					}
				}

				if (event.type == SDL_MOUSEWHEEL)
				{
					if (event.wheel.y >0)
					{
							mouseInfo->incScrollAccumulator();
							//cout << "ScrollUp" << endl;
					}
					if (event.wheel.y <0)
					{
							mouseInfo->decScrollAccumulator();
							//cout << "ScrollDown" << endl;
					}
				}
			}

}

void handlePicking()
{

	if (mouseInfo->getLMBState() == BTN_HELD)
	{
		if (mouseInfo->getLocalPosition() != lastLoc)
		{
			//cout << "m: " << mouseInfo->getLocalPosition().x << " " << mouseInfo->getLocalPosition().y << endl;
			//vec2 uvPick = getUVFromFBO(fbo_id, mouseInfo->getLocalPosition());
			//cout << "m: " << uvPick.x << " " << uvPick.y << endl;

			//cout << "uv->texCoords: " << ivec2(uvPick.s*1024, 0).x << " " << ivec2(0, 1024-(uvPick.t*1024)).y << endl;
			//setPixelInTexture(uvPick.s*1024, 1024-(uvPick.t*1024));
			plotLine(lastLoc, mouseInfo->getLocalPosition());
		}
	}
}

/*
 * **Algorithm**
- Get normalized vector from mouse positions, (last & currrent).
- While pos.x != P2.x
  - plotPixel(round(pos.x), round(pos.y))
  - pos.xy += Vn.xy
 * */
void plotLine(const ivec2 P1, const ivec2 P2)
{
	// Get normalized vector
	vec2 Vn = glm::normalize(vec2(P2)-vec2(P1));

	// Plot line
	vec2 Pn = P1;

	// Locations to plot
	vector<vec2> locations;
	vec2 uv = vec2(0);

	//glFlush();  // Testing if this is not necessary
	//glFinish(); // Testing if this is not necessary

	long time = SDL_GetTicks();
	do
	{
		uv = getUVFromFBO(fbo_id, ivec2(glm::round(Pn.x), glm::round(Pn.y)));
		locations.push_back(uv);
		Pn += Vn;
	} while (glm::round(Pn.x) != P2.x && glm::round(Pn.y) != P2.y);
	cout << "Time-Get " << locations.size() << ": " << SDL_GetTicks()-time << endl;

	time = SDL_GetTicks();
	for (vector<vec2>::const_iterator it = locations.begin(); it != locations.end(); ++it)
	{
		setPixelInTexture(it->x*1024, 1024-(it->y*1024));
		cout << "put: " << it->x << ", " << it->y << endl;
	}
	cout << "Time-Put " << locations.size() << ": " << SDL_GetTicks()-time << endl;
}

const vec2 getUVFromFBO(const GLuint fboID, const ivec2 loc)
{
	//glFlush();
	//glFinish();

	//glBindFramebuffer(GL_FRAMEBUFFER, fboID);
	//vec4 pixel = vec4(0);
	//glReadPixels(loc.x, 512-loc.y, 1, 1, GL_RGBA, GL_FLOAT, &pixel);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//return vec2(pixel.s, pixel.t);

	vec4 pixel = vec4(0);
	//glReadBuffer(fbo_id);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_id);
	glReadPixels(loc.x, 512-loc.y, 1, 1, GL_RGBA, GL_FLOAT, 0);
	//glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	cout << "Pixel: " << pixel.s << " " << pixel.t << endl;

	return ivec2(pixel.s, pixel.t);
}

void setPixelInTexture(GLint x, GLint y)
{
	GLuint  id = texDiffuse;

	uint8_t color[4];
	color[0] = 10;
	color[1] = 10;
	color[2] = 255;
	color[3] = 255;

	glBindTexture(GL_TEXTURE_2D, id);
	glTexSubImage2D(GL_TEXTURE_2D,
					0,
					x,
					y,
					1,
					1,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					(const void*)&color);

	glBindTexture(GL_TEXTURE_2D, 0);

	/*
	GLuint  id = texDiffuse;

	uint8_t r  = 10;
	uint8_t g  = 10;
	uint8_t b  = 255;
	uint8_t a  = 255;

	uint8_t data[4];
	data[0] = r;
	data[1] = g;
	data[2] = b;
	data[3] = a;
	glBindTexture(GL_TEXTURE_2D, id);
	glTexSubImage2D(GL_TEXTURE_2D,
					0,
					x,
					y,
					1,
					1,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					data);

	glBindTexture(GL_TEXTURE_2D, 0);
	*/

	cout << "Set: " << x << " " << y << endl;
}

void render()
{
	// Pass 1
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ShaderProgram* currentShader = nullptr;
	if (renderMode == 1)
	{
		currentShader = shader;
	}
	else if (renderMode == 2)
	{
		currentShader = shaderPicking;
	}

	// Update normalmatrix
	normalMatrix = transpose(inverse(mat3(viewMatrix * modelMatrix)));	// Correct??

	currentShader->bindShader();
	currentShader->sendUniform4x4("modelMatrix", modelMatrix);
	currentShader->sendUniform4x4("viewMatrix", viewMatrix);
	currentShader->sendUniform4x4("projectionMatrix", projectionMatrix);
	currentShader->sendUniform3x3("normalMatrix", normalMatrix);
	if (renderMode == 1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texDiffuse);
		currentShader->sendUniform("diffuseTextureID", 0);
	}
	renderMesh();
	currentShader->unbindShader();

	// Pass 2
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Update normalmatrix
	normalMatrix = transpose(inverse(mat3(viewMatrix * modelMatrix)));	// Correct??

	shaderPicking->bindShader();
	shaderPicking->sendUniform4x4("modelMatrix", modelMatrix);
	shaderPicking->sendUniform4x4("viewMatrix", viewMatrix);
	shaderPicking->sendUniform4x4("projectionMatrix", projectionMatrix);
	shaderPicking->sendUniform3x3("normalMatrix", normalMatrix);

	//glViewport(0, 0, 512, 512);
	renderMesh();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	shaderPicking->unbindShader();

	SDL_GL_SwapWindow(wndMain);
}

void renderMesh()
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glBindVertexArray(0);
}

bool cleanup()
{
	// Delete objects
	delete mouseInfo;

	// Destroy textures
	glDeleteTextures(1, &texDiffuse);
	glDeleteTextures(1, &fbo_tex);

	// Destroy 3d object?

	// Delete shaders
	delete shader;
	delete shaderPicking;

	// Once finished with OpenGL functions, the SDL_GLContext can be deleted.
	SDL_GL_DeleteContext(mainContext);

	//Destroy window
	SDL_DestroyWindow(wndMain);

	//Quit SDL subsystems
	SDL_Quit();

	return true;
}
