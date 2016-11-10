/*
 * ShaderProgram.h
 *
 *  Created on: 16 okt. 2016
 *      Author: bladd
 */

#ifndef CLASSES_SHADERPROGRAM_H_
#define CLASSES_SHADERPROGRAM_H_

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>

#include <GL/glew.h>	// Including glew before glut
#include <glm/glm.hpp>	// GLM, GLMath for mathematics implementation

using namespace std;
using glm::mat4;
using glm::mat3;
using glm::vec4;
using glm::vec3;
using glm::vec2;


class ShaderProgram
{
public:
    struct Shader
	{
		bool            use;	    // Determines if being used
		unsigned int    handle;     // Shader handle
		GLenum          shaderType; // Shader type enum, VS, GS, FS, etc...
		string          filename;   // The filename of shader file
		string          script;     // The shader code as string

        // Default constructor
		Shader()
		{
            // Set default data
			use 		= false;
			handle 		= 0;
			shaderType 	= 0;
			filename 	= string();
			script 		= string();
		}
	};

private:
    Shader			m_vertexShader;		// Container for vertex shader source
	Shader			m_fragmentShader;	// Container for fragment shader source
	Shader			m_tessControlShader;	// Container for tesselation control shader source
	Shader			m_tessEvaluationShader;	// Container for tesselation evaluation shader source
	Shader			m_geometryShader;	// Container for geometry shader source
	unsigned int	m_programHandle;	// ShaderProgram id (or handle)

	map<string, GLuint> m_uniformMap;	// uniform cache
	map<string, GLuint> m_attributeMap;	// attribute cache ( note: not implemeted )

	bool fIgnoreWarnings;  // Determines if warnings will be ignored. ( note: Ignores printing missing subroutines, when rendering the light sprites. )

public:
    ShaderProgram();    // Default constructor
    ~ShaderProgram();   // Destructor, calls 'unload()'

    void unload();      // Frees up memory
    void addShader(const string &filename, const GLenum &shaderType);    // Adds shader of 'type' to shader program ( note: recently changed to '&' )
    bool compile();     // Compiles shaders
    bool link();        // Links shaders into a shader program

    GLuint getUniformLocation(const string &name);  // Fetches/Generates uniform location handle
    GLuint getAttribLocation(const string &name);   // Fetches/Generates attribute location handle ( note: not implemeted )

    GLuint getSubroutine(const string &name, const GLenum &shaderType);                                 // Find/Get subroutine handle from shader
    void   setSubroutine(const GLuint &subroutine, const GLenum &shaderType);                           // Sets the subroutine in shader
    void   setSubroutines(const GLuint *subroutines, const GLenum &shaderType, const GLuint &count = 1);// Sets the multiple subroutines in shader

    void sendUniform(const string &name, const int id);                                                     // Sends integer  as uniform
    void sendUniform(const string &name, const unsigned int id);                                            // Sends unsigned integer  as uniform
    void sendUniform(const string &name, const float scalar);                                               // Sends float    as uniform
    void sendUniform(const string &name, const float r,  const float g,  const float b,  const float a);    // Sends 4 floats as uniform
    void sendUniform(const string &name, const float x,  const float y,  const float z);                    // Sends 3 floats as uniform
    void sendUniform(const string &name, const vec4 &values);
    void sendUniform(const string &name, const vec3 &values);
    void sendUniform(const string &name, const vec2 &values);
    void sendUniform4x4(const string &name, const mat4 &matrix, bool transpose = false);                    // Sends 4x4 matrix as uniform, transpose default false
    void sendUniform3x3(const string &name, const mat3 &matrix, bool transpose = false);                    // Sends 3x3 matrix as uniform

    void bindAttribute(unsigned int index, const string &attribName);   //  ( note: not implemeted )

    void bindShader();  // Select/Use shader
    void unbindShader();// deSelect shader
    GLuint& getHandle();// Returns shader id

    void ignoreWarnings();  // Sets the flag to true

private:
    string  readFile(const string &filename);       // Read shader code from file
    bool    compileShader(const Shader &shader);    // Compile shader code
    void    saveLogFile(const int &length, const Shader &shader);           // Save error log to file
    void    saveLinkLogFile(const int &length, const GLuint &programHandle);// Save error log to file
};

#endif /* CLASSES_SHADERPROGRAM_H_ */
