#version 410

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoords;
layout(location = 2) in vec3 vNormal;

layout(location=0) out vec3 vPositionOut;
layout(location=1) out vec2 vTexCoordsOut;
layout(location=2) out vec3 vNormalOut;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

mat4 modelViewMatrix;
mat4 modelViewProjectionMatrix;

void main()
{
	// Handle texCoords
	vTexCoordsOut = vTexCoords;

	// Handle normal
	vNormalOut = normalize(normalMatrix * vNormal);

	// Matrices
	modelViewMatrix = viewMatrix * modelMatrix;
	modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;

	// Handle position
	vPositionOut	= vec3((modelViewMatrix * vec4(vPosition, 1.0)).xyz);	
	gl_Position 	= modelViewProjectionMatrix * vec4(vPosition, 1.0);
} 
