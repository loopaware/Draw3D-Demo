#version 410

layout(location = 0) in vec3 vPosition;
//layout(location = 1) in vec2 vTexCoords;

layout(location=0) out vec3 vPositionOut;
//layout(location=1) out vec2 vTexCoordsOut;

void main()
{
	// Handle texCoords
	//vTexCoordsOut = vec2(1);

	// Handle position
	vPositionOut	= vPosition;	
	gl_Position 	= vec4(vPosition, 1.0);
} 
