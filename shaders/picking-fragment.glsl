#version 410

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoords;
layout(location = 2) in vec3 vNormal;

layout(location = 0) out vec4 outColor;

/// Main function
void main()
{
	outColor = vec4(vTexCoords.s, vTexCoords.t, 0, 1);
}
