/*
 * ShaderProgram.cpp
 *
 *  Created on: 16 okt. 2016
 *      Author: bladd
 */

#include "ShaderProgram.h"

ShaderProgram::ShaderProgram()
{
    // Generate handle
    m_programHandle = glCreateProgram();

    // Validate
    if (0 == m_programHandle)
    {
        printf("ERROR: Could not create shader program. ( In ShaderProgram constructor )\n");
    }

    // Set defaults
    fIgnoreWarnings = false;
}

ShaderProgram::~ShaderProgram()
{
    // Free up memory
    unload();
}

///////////////////////////////

void ShaderProgram::unload()
{
    // Detach shaders from shaderprogram
    glDetachShader(m_programHandle, m_vertexShader.handle);
    glDetachShader(m_programHandle, m_fragmentShader.handle);
    glDetachShader(m_programHandle, m_tessControlShader.handle);
    glDetachShader(m_programHandle, m_tessEvaluationShader.handle);
    glDetachShader(m_programHandle, m_geometryShader.handle);

    // Delete shaders
    glDeleteShader(m_vertexShader.handle);
    glDeleteShader(m_fragmentShader.handle);
    glDeleteShader(m_tessControlShader.handle);
    glDeleteShader(m_tessEvaluationShader.handle);
    glDeleteShader(m_geometryShader.handle);

    // Delete shader program
    glDeleteProgram(m_programHandle);
}

void ShaderProgram::addShader(const string &filename, const GLenum &shaderType)
{
    if		(shaderType == GL_VERTEX_SHADER)
    {
        m_vertexShader.handle		= glCreateShader(GL_VERTEX_SHADER);
        m_vertexShader.filename		= filename;
        m_vertexShader.script		= readFile(m_vertexShader.filename);
        m_vertexShader.use			= true;
    }
    else if (shaderType == GL_FRAGMENT_SHADER)
    {
        m_fragmentShader.handle		= glCreateShader(GL_FRAGMENT_SHADER);
        m_fragmentShader.filename	= filename;
        m_fragmentShader.script		= readFile(m_fragmentShader.filename);
        m_fragmentShader.use		= true;
    }
    else if (shaderType == GL_TESS_CONTROL_SHADER)
    {
        m_tessControlShader.handle		= glCreateShader(GL_TESS_CONTROL_SHADER);
        m_tessControlShader.filename	= filename;
        m_tessControlShader.script		= readFile(m_geometryShader.filename);
        m_tessControlShader.use		    = true;
    }
    else if (shaderType == GL_TESS_EVALUATION_SHADER)
    {
        m_tessEvaluationShader.handle	= glCreateShader(GL_TESS_EVALUATION_SHADER);
        m_tessEvaluationShader.filename	= filename;
        m_tessEvaluationShader.script	= readFile(m_geometryShader.filename);
        m_tessEvaluationShader.use		= true;
    }
    else if (shaderType == GL_GEOMETRY_SHADER)
    {
        m_geometryShader.handle		= glCreateShader(GL_GEOMETRY_SHADER);
        m_geometryShader.filename	= filename;
        m_geometryShader.script		= readFile(m_geometryShader.filename);
        m_geometryShader.use		= true;
    }
}

bool ShaderProgram::compile()
{
    //// compile shaders
    // if shader is assigned, compile
    if (m_vertexShader.use)		// Vertex shader
    {
        if (!compileShader(m_vertexShader))			// Compile shader
        {
            cout << "ERROR: Could not compile vertex shader file:" << m_vertexShader.filename;
            return false;
        }
    }

    if (m_fragmentShader.use)	// Fragment shader
    {
        if (!compileShader(m_fragmentShader)) // Compile shader
        {
            cout << "ERROR: Could not compile fragment shader file:" << m_fragmentShader.filename;
            return false;
        }
    }

    if (m_tessControlShader.use)	// tesselation control shader
    {
        if (!compileShader(m_tessControlShader))		// Compile shader
        {
            cout << "ERROR: Could not compile tesselation control shader file:" << m_tessControlShader.filename;
            return false;
        }
    }

    if (m_tessEvaluationShader.use)	// tesselation evaluation shader
    {
        if (!compileShader(m_tessEvaluationShader))		// Compile shader
        {
            cout << "ERROR: Could not compile tesselation evaluation shader file:" << m_tessEvaluationShader.filename;
            return false;
        }
    }

    if (m_geometryShader.use)	// Geometry shader
    {
        if (!compileShader(m_geometryShader))		// Compile shader
        {
            cout << "ERROR: Could not compile geometry shader file:" << m_geometryShader.filename;
            return false;
        }
    }


    // Attach shaders
    if (m_vertexShader.use)		// Vertex shader
    {
        glAttachShader(m_programHandle, m_vertexShader.handle);
    }
    if (m_fragmentShader.use)	// Fragment shader
    {
        glAttachShader(m_programHandle, m_fragmentShader.handle);
    }
    if (m_tessControlShader.use)	// tesselation control shader
    {
        glAttachShader(m_programHandle, m_tessControlShader.handle);
    }
    if (m_tessEvaluationShader.use)	// tesselation evaluation shader
    {
        glAttachShader(m_programHandle, m_tessEvaluationShader.handle);
    }
    if (m_geometryShader.use)	// Geometry shader
    {
        glAttachShader(m_programHandle, m_geometryShader.handle);
    }

    return true;
}

bool ShaderProgram::link()
{
    // Create shader program
    if (0 == m_programHandle)
    {
        printf("ERROR: Could not create shader program. ( While linking )\n");
        return false;
    }

    // Link program
    glLinkProgram(m_programHandle);

    // Verify linking
    GLint status;
    glGetProgramiv(m_programHandle, GL_LINK_STATUS, &status);
    if (GL_FALSE == status)
    {
        printf("ERROR: Could not link shader program.\n");

        int length = 0;
        glGetProgramiv(m_programHandle, GL_INFO_LOG_LENGTH, &length);
        if (length > 0)
        {
            cout << "Log saved!";
            saveLinkLogFile(length, m_programHandle);
        }

        return false;
    }

    return true;
}

///////////////////////////////

GLuint ShaderProgram::getUniformLocation(const string &name)
{
    map<string, GLuint>::iterator i = m_uniformMap.find(name);
    if (i == m_uniformMap.end())
    {
        GLuint location = glGetUniformLocation(m_programHandle, name.c_str());
        m_uniformMap.insert(std::make_pair(name, location));
        return location;
    }

    return (*i).second;
}

GLuint ShaderProgram::getAttribLocation(const string &name)
{
    map<string, GLuint>::iterator i = m_attributeMap.find(name);
    if (i == m_attributeMap.end())
    {
        GLuint location = glGetAttribLocation(m_programHandle, name.c_str());
        m_attributeMap.insert(std::make_pair(name, location));
        return location;
    }

    return (*i).second;
}
///////////////////////////////

GLuint ShaderProgram::getSubroutine(const string &name, const GLenum &shaderType)
{
    // Fetch id
    GLuint id = glGetSubroutineIndex(m_programHandle, shaderType, name.c_str());

    // Validate id
    if (id == GL_INVALID_INDEX)
    {
        if (!fIgnoreWarnings)
            cout << "WARNING: Subroutine name '" << name << "' " << "Was not found!" << endl;
    }

    // return handle/id
    return id;
}

void ShaderProgram::setSubroutine(const GLuint &subroutine, const GLenum &shaderType)
{
    // Set shader subroutine
    glUniformSubroutinesuiv(shaderType, 1, &subroutine);
}

void ShaderProgram::setSubroutines(const GLuint *subroutines, const GLenum &shaderType, const GLuint &count)
{
    // Set shader subroutines
    glUniformSubroutinesuiv(shaderType, count, subroutines);
}
///////////////////////////////

void ShaderProgram::sendUniform(const string &name, const int id)
{
    GLuint location = getUniformLocation(name);
    glUniform1i(location, id);
}

void ShaderProgram::sendUniform(const string &name, const unsigned int id)
{
    GLuint location = getUniformLocation(name);
    glUniform1i(location, id);
}

void ShaderProgram::sendUniform(const string &name, const float scalar)
{
    GLuint location = getUniformLocation(name);
    glUniform1f(location, scalar);
}

void ShaderProgram::sendUniform(const string &name, const float r,  const float g,  const float b,  const float a)
{
    GLuint location = getUniformLocation(name);
    glUniform4f(location, r, g, b, a);
}

void ShaderProgram::sendUniform(const string &name, const float x,  const float y,  const float z)
{
    GLuint location = getUniformLocation(name);
    glUniform3f(location, x, y, z);
}

void ShaderProgram::sendUniform(const string &name, const vec4 &values)
{
    GLuint location = getUniformLocation(name);
    glUniform4f(location, values.x, values.y, values.z, values.w);
}

void ShaderProgram::sendUniform(const string &name, const vec3 &values)
{
    GLuint location = getUniformLocation(name);
    glUniform3f(location, values.x, values.y, values.z);
}

void ShaderProgram::sendUniform(const string &name, const vec2 &values)
{
    GLuint location = getUniformLocation(name);
    glUniform2f(location, values.x, values.y);
}

void ShaderProgram::sendUniform4x4(const string &name, const mat4 &matrix, bool transpose)
{
    GLuint location = getUniformLocation(name);
    glUniformMatrix4fv(location, 1, transpose, &matrix[0][0]);
}

void ShaderProgram::sendUniform3x3(const string &name, const mat3 &matrix, bool transpose)
{
    GLuint location = getUniformLocation(name);
    glUniformMatrix3fv(location, 1, transpose, &matrix[0][0]);
}

///////////////////////////////

void ShaderProgram::bindAttribute(unsigned int index, const string &attribName)
{
    glBindAttribLocation(m_programHandle, index, attribName.c_str());
}

///////////////////////////////

void ShaderProgram::bindShader()
{
    glUseProgram(m_programHandle);
}

void ShaderProgram::unbindShader()
{
    glUseProgram(0);
}

GLuint& ShaderProgram::getHandle()
{
    return m_programHandle;
}

void ShaderProgram::ignoreWarnings()
{
    fIgnoreWarnings = true;
}

///////////////////////////////

string ShaderProgram::readFile(const string &filename)
{
    ifstream fileIn(filename.c_str());

    if (!fileIn.good())
    {
        cout << "Error: Could not open shader file: " << filename << endl;
        return string();
    }

    string stringBuffer(istreambuf_iterator<char>(fileIn), (istreambuf_iterator<char>()));

    return stringBuffer;
}

bool ShaderProgram::compileShader(const Shader &shader)
{
    // Load source from string
    const char* ptr = shader.script.c_str();
    glShaderSource(shader.handle, 1, &ptr, NULL);

    // Compile shader
    glCompileShader(shader.handle);

    // Check for errors
    int result = 0;
    glGetShaderiv(shader.handle, GL_COMPILE_STATUS, &result);
    if (GL_FALSE == result)
    {
        //printf("ERROR: Could not compile shader of type %s.\n", shader.shaderType);
        int length = 0;
        glGetShaderiv(shader.handle, GL_INFO_LOG_LENGTH, &length);
        if (length > 0)
        {
            saveLogFile(length, shader);
        }
        return false;
    }

    return true;
}

void ShaderProgram::saveLogFile(const int &length, const Shader &shader)
{
    // Create and print error log
    char* errorLog = new char[length];
    int written = 0;
    glGetShaderInfoLog(shader.handle, length, &written, errorLog);
    printf("Shader error log:\n%s\n", errorLog);

    // save log to file
    ofstream hLog;
    hLog.open("errlog_shader_compile.txt");
    if (!hLog.is_open())
    {
        printf("\nERROR: Could not save error log. ( Shader: %s )\n\n", shader.filename.c_str());
    }
    hLog.write(errorLog, length);
    hLog.close();

    delete [] errorLog; // Redundant?
}

void ShaderProgram::saveLinkLogFile(const int &length, const GLuint &programHandle)
{
    // Create and print error log
    char* errorLog = new char[length];
    int written = 0;
    glGetProgramInfoLog(programHandle, length, &written, errorLog);
    printf("Shader error log:\n%s\n", errorLog);

    // save log to file
    ofstream hLog;
    hLog.open("errlog_shader_link.txt");
    if (!hLog.is_open())
    {
        printf("\nERROR: Could not save error log.\n\n");
    }
    hLog.write(errorLog, length);
    hLog.close();

    delete [] errorLog; // Redundant?
}
